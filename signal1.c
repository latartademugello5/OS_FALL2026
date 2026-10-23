#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

int handled = 0; // Global flag to track signal delivery

void handler(int signum) {
    printf("Hello World!\n");
    handled = 1; // Indicate that the handler was executed
}

int main(int argc, char * argv[]) {
    signal(SIGALRM, handler); // Register signal handler
    alarm(5); // Schedule a SIGALRM signal in 5 seconds

    while(!handled); // Busy wait until the signal handler sets the flag

    printf("Turing was right!\n");
    return 0;
}