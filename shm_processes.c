#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>

void DearOldDad(int ShmPTR[]);
void PoorStudent(int ShmPTR[]);

int main(int argc, char *argv[])
{
    int ShmID;
    int *ShmPTR;
    pid_t pid;
    int status;

    // Request shared memory for 2 integers:
    // ShmPTR[0] = BankAccount
    // ShmPTR[1] = Turn
    ShmID = shmget(IPC_PRIVATE, 2 * sizeof(int), IPC_CREAT | 0666);
    if (ShmID < 0) {
        printf("*** shmget error (server) ***\n");
        exit(1);
    }

    ShmPTR = (int *) shmat(ShmID, NULL, 0);
    if ((long) ShmPTR == -1) {
        printf("*** shmat error (server) ***\n");
        exit(1);
    }

    // Initialize shared variables to 0
    ShmPTR[0] = 0; // BankAccount
    ShmPTR[1] = 0; // Turn

    pid = fork();
    if (pid < 0) {
        printf("*** fork error (server) ***\n");
        exit(1);
    }
    else if (pid == 0) {
        PoorStudent(ShmPTR);
        exit(0);
    }
    else {
        DearOldDad(ShmPTR);
    }

    wait(&status);
    shmdt((void *) ShmPTR);
    shmctl(ShmID, IPC_RMID, NULL);
    exit(0);
}

void DearOldDad(int ShmPTR[])
{
    srandom(time(NULL) ^ (getpid() << 16));
    
    for (int i = 0; i < 25; i++) {
        // Sleep random amount of time between 0 - 5 seconds
        sleep(random() % 6);

        // Strict alternation loop: wait while Turn != 0
        while (ShmPTR[1] != 0) {
            // no-op
        }

        int account = ShmPTR[0];

        if (account <= 100) {
            // Deposit Money
            int balance = random() % 101; // Random amount between 0 - 100
            
            if (balance % 2 == 0) {
                account += balance;
                printf("Dear old Dad: Deposits $%d / Balance = $%d\n", balance, account);
            } else {
                printf("Dear old Dad: Doesn't have any money to give\n");
            }
        } else {
            printf("Dear old Dad: Thinks Student has enough Cash ($%d)\n", account);
        }

        // Copy value back and update Turn
        ShmPTR[0] = account;
        ShmPTR[1] = 1;
    }
}

void PoorStudent(int ShmPTR[])
{
    srandom(time(NULL) ^ (getpid() << 16));

    for (int i = 0; i < 25; i++) {
        // Sleep random amount of time between 0 - 5 seconds
        sleep(random() % 6);

        // Strict alternation loop: wait while Turn != 1
        while (ShmPTR[1] != 1) {
            // no-op
        }

        int account = ShmPTR[0];
        int balance = random() % 51; // Random amount between 0 - 50

        printf("Poor Student needs $%d\n", balance);

        if (balance <= account) {
            account -= balance;
            printf("Poor Student: Withdraws $%d / Balance = $%d\n", balance, account);
        } else {
            printf("Poor Student: Not Enough Cash ($%d)\n", account);
        }

        // Copy value back and update Turn
        ShmPTR[0] = account;
        ShmPTR[1] = 0;
    }
}