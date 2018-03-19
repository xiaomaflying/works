#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>

int main(void)
{
    printf("USR1: %d\n", SIGUSR1);
    printf("USR2: %d\n", SIGUSR2);
    printf("SA_RESTART: %d\n",  SA_RESTART);
    return 0;
}

