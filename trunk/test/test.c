#include <yajl/yajl_parse.h>
#include <yajl/yajl_gen.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int phrase(void * ctx, const unsigned char * stringVal, unsigned int stringLen)
{
	char phrase_buf[256];
	strncpy(phrase_buf, stringVal, stringLen);
	printf("phrase=\"%s\"\n\n", phrase_buf);
    return 1;
}

static int id(void * ctx, const unsigned char * stringVal, unsigned int stringLen)
{
	char id_buf[256];
	strncpy(id_buf, stringVal, stringLen);
	id_buf[stringLen] = '\0';
	printf("id=\"%s\"", id_buf);
    return 1;
}

static yajl_callbacks callbacks = {NULL, NULL, NULL, NULL, NULL, phrase, NULL, id, NULL, NULL, NULL};

int main(int argc, char ** argv)
{
    yajl_handle hand;
    yajl_status stat;
    yajl_parser_config cfg = { 1, 1 };

    int done = 0;

    hand = yajl_alloc(&callbacks, &cfg, NULL, (void *) g);
        
    while (!done) {
        rd = fread((void *) fileData, 1, sizeof(fileData) - 1, stdin);
        
        if (rd == 0) {
            if (!feof(stdin)) {
                fprintf(stderr, "error on file read.\n");
                break;
            }
            done = 1;
        }
        fileData[rd] = 0;
        
        if (done)
            /* parse any remaining buffered data */
            stat = yajl_parse_complete(hand);
        else
            /* read file data, pass to parser */
            stat = yajl_parse(hand, fileData, rd);

        if (stat != yajl_status_ok &&
            stat != yajl_status_insufficient_data)
        {
            unsigned char * str = yajl_get_error(hand, 1, fileData, rd);
            fprintf(stderr, (const char *) str);
            yajl_free_error(hand, str);
        } else {
            const unsigned char * buf;
            unsigned int len;
            yajl_gen_get_buf(g, &buf, &len);
            fwrite(buf, 1, len, stdout);
            yajl_gen_clear(g);
        }
    }

    yajl_gen_free(g);
    yajl_free(hand);
    
    return 0;
}
