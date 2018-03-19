#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv)
{
    int c, i;

    if (argc != 2 || strlen(argv[1]) != 1) {
	fprintf(stderr, "usage: %s character\n", argv[0]);
	return(1);
    }
    c = argv[1][0];

    for (i = 0; i < 8; i++) {
	if (c & 128)
	    printf("1\n");
	else
	    printf("0\n");
	c <<= 1;
    }

    return(0);
}
