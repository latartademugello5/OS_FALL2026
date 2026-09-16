#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>

void child_process() {
    // Seed the random number generator uniquely for each child process using PID and time
    srandom(time(NULL) ^ getpid());

    // Loop a random number of iterations (up to 30)
    int iterations = random() % 31; // 0 to 30 iterations

    for (int i = 0; i < iterations; i++) {
        pid_t pid = getpid();
        pid_t ppid = getppid();

        printf("Child Pid: %d is going to sleep!\n", pid);

        // Sleep for a random amount of time, up to 10 seconds
        int sleep_time = random() % 11; // 0 to 10 seconds
        sleep(sleep_time);

        printf("Child Pid: %d is awake!\nWhere is my Parent: %d?\n", pid, ppid);
    }

    exit(0);
}

int main() {
    pid_t pid1, pid2;

    // Fork the first child process
    pid1 = fork();

    if (pid1 < 0) {
        perror("Fork failed");
        exit(1);
    } else if (pid1 == 0) {
        // First child process
        child_process();
    }

    // Fork the second child process
    pid2 = fork();

    if (pid2 < 0) {
        perror("Fork failed");
        exit(1);
    } else if (pid2 == 0) {
        // Second child process
        child_process();
    }

    // Parent Process: Wait for both child processes to complete
    int status;
    for (int i = 0; i < 2; i++) {
        pid_t completed_pid = wait(&status);
        printf("Child Pid: %d has completed\n", completed_pid);
    }

    return 0;
}