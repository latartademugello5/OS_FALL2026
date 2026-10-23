#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>

int alarm_count = 0;
time_t start_time;

void sigalrm_handler(int signum) {
    alarm_count++;
    alarm(1); // Re-arm the alarm for 1 second
}

void sigint_handler(int signum) {
    time_t end_time = time(NULL);
    int total_seconds = (int)difftime(end_time, start_time);

    printf("\nProgram executed for %d seconds.\n", total_seconds);
    printf("Total alarms delivered: %d\n", alarm_count);
    exit(0);
}

int main(int argc, char * argv[]) {
    start_time = time(NULL); // Record start time

    signal(SIGALRM, sigalrm_handler); // Register handler for SIGALRM
    signal(SIGINT, sigint_handler);   // Register handler for SIGINT (CTRL-C)

    alarm(1); // Schedule the first SIGALRM in 1 second

    while (1) {
        pause(); // Wait for any signal to arrive
    }

    return 0;
}