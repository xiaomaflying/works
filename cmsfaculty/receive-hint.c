/*
 * This shows waiting for a signal, as you will have to do in receive.c.
 * Actually, it waits for TWO signals -- you have to kill this program twice.
 */

#include <stdio.h>
#include <unistd.h>
#include <signal.h>

int signalcount = 0;

int main()
{
    struct sigaction sa;
    extern void myhandler();

    sa.sa_handler = myhandler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(15, &sa, NULL);

    printf("My pid is %ld\n", (long)getpid());
    while (signalcount < 2)
	sleep(1234);  /* will be interrupted by signal */
    printf("Ack!\n");
    return(0);
}


void myhandler()
{
    signalcount++;
}
