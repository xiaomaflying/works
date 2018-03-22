#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>

int main(void)
{
	char string[] = "1234";
	int sum = atoi( string );
	printf("Sum = %d\n", sum ); // Outputs: Sum = 1234

    printf("USR1: %d\n", SIGUSR1);
    printf("USR2: %d\n", SIGUSR2);
    printf("SA_RESTART: %d\n",  SA_RESTART);
    return 0;
}

