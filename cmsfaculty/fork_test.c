#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main()
{
    int sum, i, status;
    extern void doit(int from);

    for (i = 0; i < 5; i++) {
	switch (fork()) {
	case -1:
	    perror("fork");
	    return(1);
	case 0:
            doit(i * 5);  /* does not return */
	}
	/* else continue around the loop */
    }
    for (sum = i = 0; i < 5; i++) {
        wait(&status);
        sum += WEXITSTATUS(status);
    }
    printf("%d\n", sum);
    return(0);
}


void doit(int from)
{
    int j, sum = 0;
    for (j = 0; j < 5; j++)
        sum += from + j;
    exit(sum);
}
