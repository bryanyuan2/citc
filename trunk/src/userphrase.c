/**
 * userphrase.c
 *
 * Copyright (c) 1999, 2000, 2001
 *	Lu-chuan Kung and Kang-pen Chen.
 *	All rights reserved.
 *
 * Copyright (c) 2004, 2006
 *	libchewing Core Team. See ChangeLog for details.
 *
 * See the file "COPYING" for information on usage and redistribution
 * of this file.
 *
 * Oct. 2010: the changes made to 0.3.2 is only for prototyping purposes only
 */

#include <stdlib.h>
#include <time.h>
#include <string.h>

#include "chewing-utf8-util.h"
#include "global.h"
#include "hash-private.h"
#include "dict-private.h"
#include "tree-private.h"
#include "userphrase-private.h"
#include "private.h"

/* these break portability */
#include <curl/curl.h>
#include <yajl/yajl_parse.h>
#include <yajl/yajl_gen.h>

#define SERVER_URL "http://citc.cse.tw/"
#define PAGE_ENTRIES 10
#define USER_REPORT 1

#define JSON_NULL 0
#define JSON_RECVD_KEY 1
#define JSON_RECVD_PHRASES 2

#define LOOKUP 0 
#define UPDATE 1
#define INSERT 2

extern int chewing_lifetime;
static HASH_ITEM *pItemLast;
CURL *curl;
UserPhraseData entry[PAGE_ENTRIES];
char entry_id[PAGE_ENTRIES][64], json_flag = 0;
int entries = 0, entry_pos = 0;
unsigned int counts=1;

#if 0
static int DeltaFreq( int recentTime )
{
	int diff;

	diff = ( chewing_lifetime - recentTime );

	if ( diff < 1000 )
		return ( 1500 - diff ); /* 1500 ~ 500 */
	if ( diff < 2000 )
		return ( 500 );       /* 500 ~ 500 */
	if ( diff < 3000 )
		return ( 2500 - diff ); /* 500 ~ -500 */
	return ( -500 );    /* -500 forever */
}
#endif

int phoneSeqLen(const uint16 phoneSeq[])
{
	int len = 0;
	while(phoneSeq[len++]);
	return len - 1;
}

/* load the orginal frequency from the static dict */
static int LoadOriginalFreq( const uint16 phoneSeq[], const char wordSeq[], int len )
{
	int pho_id;
	int retval;
	Phrase *phrase = ALC( Phrase, 1 );

	pho_id = TreeFindPhrase( 0, len - 1, phoneSeq );
	if ( pho_id != -1 ) {
		GetPhraseFirst( phrase, pho_id );
		do {
			/* find the same phrase */
			if ( ! strcmp(
				phrase->phrase, 
				wordSeq ) ) {
				retval = phrase->freq;	
				free( phrase );
				return retval;
			}
		} while ( GetPhraseNext( phrase ) );
	}

	free( phrase );
	return FREQ_INIT_VALUE;
}

/* find the maximum frequency of the same phrase */
static int LoadMaxFreq( const uint16 phoneSeq[], int len )
{
	int pho_id;
	Phrase *phrase = ALC( Phrase, 1 );
	int maxFreq = FREQ_INIT_VALUE;
	UserPhraseData *uphrase;

	pho_id = TreeFindPhrase( 0, len - 1, phoneSeq );
	if ( pho_id != -1 ) {
		GetPhraseFirst( phrase, pho_id );
		do {
			if ( phrase->freq > maxFreq )
				maxFreq = phrase->freq;
		} while( GetPhraseNext( phrase ) );
	}
	free( phrase );

	// retrieve max freq from server
	uphrase = UserGetPhraseFirst( phoneSeq );
	while ( uphrase ) {
		if ( uphrase->userfreq > maxFreq )
			maxFreq = uphrase->userfreq;
		uphrase = UserGetPhraseNext( phoneSeq );
	}	  

	return maxFreq;
}

/* compute the new updated freqency */
static int UpdateFreq( int freq, int maxfreq, int origfreq, int deltatime )
{
	int delta;

	/* Short interval */
	if ( deltatime < 4000 ) {
		delta = ( freq >= maxfreq ) ?
			min( 
				( maxfreq - origfreq ) / 5 + 1, 
				SHORT_INCREASE_FREQ ) :
			max( 
				( maxfreq - origfreq ) / 5 + 1, 
				SHORT_INCREASE_FREQ );
		return min( freq + delta, MAX_ALLOW_FREQ );
	}
	/* Medium interval */
	else if ( deltatime < 50000 ) {
		delta = ( freq >= maxfreq ) ?
			min( 
				( maxfreq - origfreq ) / 10 + 1, 
				MEDIUM_INCREASE_FREQ ) :
			max( 
				( maxfreq - origfreq ) / 10 + 1, 
				MEDIUM_INCREASE_FREQ );
		return min( freq + delta, MAX_ALLOW_FREQ );
	}
	/* long interval */
	else {
		delta = max( ( freq - origfreq ) / 5, LONG_DECREASE_FREQ );
		return max( freq - delta, origfreq );
	}
}

/* yajl callback function for parsing phrase in JSON */
static int phrase(void * ctx, const unsigned char * stringVal, unsigned int stringLen)
{
	if(json_flag == 0)
	{
		char crc_key[256];
		strncpy(crc_key, stringVal, stringLen);
/*                printf("Retrieved page for \"%s\"\n", crc_key);*/
		json_flag = JSON_RECVD_KEY;
	}
	else
	{
		strncpy(entry[entries].wordSeq, stringVal, stringLen);
/*                printf(", phrase: \"%s\"\n", entry[entries].wordSeq);*/
		++entries; // only after parsing the phrase
		json_flag = JSON_RECVD_PHRASES;
	}

    return 1;
}

/* yajl callback function for parsing id in JSON */
static int id(void * ctx, const unsigned char * stringVal, unsigned int stringLen)
{
    strncpy(entry_id[entries], stringVal, stringLen);
/*    printf("id: \"%s\"", entry_id[entries]);*/
    return 1;
}

static yajl_callbacks callbacks = {NULL, NULL, NULL, NULL, NULL, phrase, NULL, id, NULL, NULL, NULL};

/* curl callback function */
size_t post_callback(void *ptr, size_t size, size_t nmemb, void *userdata)
{
	char buf[1024];
	yajl_handle hand;
	yajl_status stat;
	yajl_parser_config cfg = {1, 1};

	memcpy(buf, ptr, size * nmemb);
	buf[size * nmemb] = '\0';
/*        printf("post_callback: %s\n",buf);*/
	// process json here
	hand = yajl_alloc(&callbacks, &cfg,  NULL, NULL);
	stat = yajl_parse(hand, buf, size * nmemb);

	if (stat != yajl_status_ok && stat != yajl_status_insufficient_data)
	{
		unsigned char *str = yajl_get_error(hand, 1, buf, size * nmemb);
		fprintf(stderr, (const char *) str);
		yajl_free_error(hand, str);
	}

	yajl_free(hand);

	return 0;
}

/* dont output if no server reply */
size_t post_quiet_callback(void *ptr, size_t size, size_t nmemb, void *userdata)
{
	return 0;
}

/*
 * send request to remote server
 * TODO: persistent connection
 *
 * */
int sendPostCurl(int op, char *key, const char wordSeq[]) {
	CURLcode res;
	char postbuf[1024];
	char *server_url;
	FILE* fpn;
	char *buf;
	if(!curl)
		curl = curl_easy_init();

	if(curl)
	{
		if( getenv("CHEWING_USER_FEEDBACK") != "null" ){
			switch(op)
			{
				case LOOKUP: 
					sprintf(postbuf, "op=0&key=%s",key); 
					break;
				case UPDATE: 
					sprintf(postbuf, "op=1&choice=%s", key); 
					break;
				case INSERT: 
					sprintf(postbuf, "op=2&key=%s&insert=%s", key, wordSeq);	
					break;
				default: 
					break;
			}
		}
		else{
			switch(op)
			{
				case LOOKUP: 
					sprintf(postbuf, "op=0&key=%s", key); 
					break;
				default: 
					break;
			}
		}
		curl_easy_setopt(curl, CURLOPT_URL, getenv("CHEWING_SERVER_URL") ? getenv("CHEWING_SERVER_URL") : SERVER_URL);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postbuf);
/*                printf("postbuf: %s\n",postbuf);*/
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, op == LOOKUP ? post_callback : post_quiet_callback);
		res = curl_easy_perform(curl);

		return 0;
	}
	else
		return -1;
}

/* server database query */
int curlLookup(const uint16 phoneSeq[])
{
	char code_buf[phoneSeqLen(phoneSeq) * sizeof(uint16)], key[256];
	KenCodeFromUint(code_buf, phoneSeq);
	getCrc64(key, code_buf);
	return sendPostCurl(LOOKUP, key, NULL);
}

/*
 * update phrase frequency
 * we need a new method of computing the frequencies
 * with the backend server, for now this only replies
 * once every selected choice
 */
int curlUpdate(const char wordSeq[])
{
	int i;

	for(i = 0 ; i < PAGE_ENTRIES; ++i)
	{
		printf("%s\n",wordSeq);
		if(strcmp(wordSeq, entry[i].wordSeq) == 0) {
			break;
		}
	}

	return sendPostCurl(UPDATE, entry_id[i], NULL);
}

/* insert phone/phrase mapping */
int curlInsert(const uint16 phoneSeq[], const char wordSeq[])
{
	char code[256];

	KenCodeFromUint(code, phoneSeq);

	return sendPostCurl(INSERT, code, wordSeq);
}

/* update frequency with backend */
int UserUpdatePhrase( const uint16 phoneSeq[], const char wordSeq[] )
{
	char buf[256];
	KenCodeFromUint(buf, phoneSeq);
/*        printf("%s\n",wordSeq);*/
	if(json_flag == JSON_RECVD_PHRASES)
	{
		if(curlUpdate(wordSeq) < 0)
		{
			printf("USER_UPDATE_FAIL: %s, %s\n", buf, wordSeq);
			return USER_UPDATE_FAIL;
		}
		else
		{
			printf("USER_UPDATE_MODIFY: %s, %s\n", buf, wordSeq);
			return USER_UPDATE_MODIFY;
		}
	}
	else
	{
		if(curlInsert(phoneSeq, wordSeq) < 0)
		{
			printf("USER_UPDATE_FAIL: %s, %s\n", buf, wordSeq);
			return USER_UPDATE_FAIL;
		}
		else
		{
			printf("USER_UPDATE_INSERT: %s, %s\n", buf, wordSeq);
			return USER_UPDATE_INSERT;
		}
	}
}

/*
 * return the first entry in page of PAGE_ENTRIES entries retrieved from backend
 * return the same if phone query does not change
 * fallback to Hash if phrase length less than 2 words
 * return NULL if backend transaction fails
 */
UserPhraseData *UserGetPhraseFirst( const uint16 phoneSeq[] )
{
	int i, len = phoneSeqLen(phoneSeq);
	char buf[256];
	KenCodeFromUint(buf, phoneSeq);
/*        printf("UserGetPhraseFirst: %s\n", buf);*/


	if(len < 2)
	{
		json_flag = JSON_NULL;
		entries = 0;
		entry_pos = 0;

		pItemLast = HashFindPhonePhrase(phoneSeq, NULL);
		if (!pItemLast)
			return NULL;
		return &(pItemLast->data);
	}
	else {
		if(!PhoneSeqTheSame(entry[0].phoneSeq, phoneSeq))
		{
			json_flag = JSON_NULL;
			entries = 0;
			entry_pos = 0;
			for(i = 0; i < PAGE_ENTRIES; ++i)
			{
				if(entry[i].phoneSeq != NULL)
					free(entry[i].phoneSeq);

				entry[i].phoneSeq = (uint16 *)calloc(len, sizeof(uint16));

				memcpy(entry[i].phoneSeq, phoneSeq, len * sizeof(uint16));

				if(entry[i].wordSeq != NULL)
					free(entry[i].wordSeq);

				entry[i].wordSeq = (char *)calloc(256, sizeof(char));
				// we don't have frequency information from the backend yet:
				srand(time(0));
				entry[i].maxfreq = MAX_ALLOW_FREQ;
				entry[i].origfreq = 1;
				entry[i].recentTime = chewing_lifetime - 1;
				entry[i].userfreq = MAX_ALLOW_FREQ;
			}

			if(curlLookup(phoneSeq) < 0 || entries == 0)
			{
/*                                printf("Lookup Failed.\n");*/
				pItemLast = HashFindPhonePhrase(phoneSeq, NULL);
				if (!pItemLast)
					return NULL;
				return &(pItemLast->data);
			}
		}
	}
	return &(entry[0]);
}

/*
 * return the next entry denoted by entry_pos in page of PAGE_ENTRIES entries retrieved from backend
 * return NULL if no more entries on page (for the time being)
 * should write page flipping mechanism for updating with backend
 */

UserPhraseData *UserGetPhraseNext( const uint16 phoneSeq[] ) 
{
	int i;

	char buf[256];
	KenCodeFromUint(buf, phoneSeq);
/*        printf("UserGetPhraseNext: %s\n", buf);*/

	if(PhoneSeqTheSame(entry[0].phoneSeq, phoneSeq))
		return entry_pos < PAGE_ENTRIES ? &(entry[entry_pos++]) : NULL;
	else
	{
		pItemLast = HashFindPhonePhrase(phoneSeq, pItemLast);
		if (!pItemLast)
			return NULL;
		return &(pItemLast->data);
	}
}

