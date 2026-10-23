#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

int handled = 0;

void handler(int signum) {
    printf("Hello World!\n");
    handled = 1;
}

int main(int argc, char * argv[]) {
    signal(SIGALRM, handler); // Register signal handler

    while (1) {
        handled = 0;
        alarm(5); // Schedule SIGALRM in 5 seconds

        while (!handled); // Busy wait until signal is handled

        printf("Turing was right!\n");
    }

    return 0;
}