#include "key2pho.c"
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char **argv)
{
	printf("%ld", UintFromPhone(argv[1]));

	return 0;
}
