/*This example is for demonstrating padding. You may try to add __attribute__((__packed__)) to prohibit padding*/
#include<stdio.h>

struct info{
	char c;
	int i;
	short j;
};

/*
struct __attribute__((__packed__)) info{
	char c;
	int i;
	short j;
};
*/

void main(){
	printf("size of struct: %ld\n", sizeof(struct info));
}
