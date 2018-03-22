#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h> 


int main(int argc, char **argv)
{
    int c, i;
    int pid;

    if ((argc != 3) || (strlen(argv[2]) != 1)){
		fprintf(stderr, "usage: pid character\n");
		return(1);
    }
    
    c = argv[2][0];
    pid = atoi(argv[1]);

    for (i = 0; i < 8; i++) {
		if (c & 128) {
			kill(pid, SIGUSR2);
		}
		else {
			kill(pid, SIGUSR1);
		}
		c <<= 1;
		usleep(100000);
    }

    return(0);
}
