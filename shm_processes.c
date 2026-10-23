#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <fcntl.h>
#include <time.h>
#include <signal.h>

int shm_id;
int *BankAccount = NULL;
sem_t *mutex = NULL;

void cleanup_and_exit(int sig) {
    if (mutex != NULL) {
        sem_close(mutex);
        sem_unlink("/bank_mutex");
    }
    if (BankAccount != NULL) {
        shmdt(BankAccount);
        shmctl(shm_id, IPC_RMID, NULL);
    }
    printf("\nProgram terminated gracefully.\n");
    exit(0);
}

int main(int argc, char *argv[]) {
    int mom_enabled = 0;
    int num_children = 1;

    // Parse command line arguments for extra credit (e.g., ./shm_processes 1 3)
    if (argc >= 2) {
        mom_enabled = atoi(argv[1]);
    }
    if (argc >= 3) {
        num_children = atoi(argv[2]);
    }

    signal(SIGINT, cleanup_and_exit);
    srand(time(NULL) ^ (getpid() << 16));

    // 1. Allocate Shared Memory for BankAccount
    shm_id = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | 0666);
    if (shm_id < 0) {
        perror("shmget error");
        exit(1);
    }

    // Attach shared memory
    BankAccount = (int *)shmat(shm_id, NULL, 0);
    if (BankAccount == (int *)-1) {
        perror("shmat error");
        exit(1);
    }

    *BankAccount = 0; // Initial balance

    // 2. Initialize Named Semaphore for Mutual Exclusion
    sem_unlink("/bank_mutex");
    mutex = sem_open("/bank_mutex", O_CREAT | O_EXCL, 0644, 1);
    if (mutex == SEM_FAILED) {
        perror("sem_open error");
        exit(1);
    }

    // 3. Fork Dear Old Dad
    pid_t dad_pid = fork();
    if (dad_pid < 0) {
        perror("fork dad failed");
        exit(1);
    } else if (dad_pid == 0) {
        // --- DEAR OLD DAD PROCESS ---
        while (1) {
            sleep(rand() % 6); // Sleep 0-5 seconds
            printf("Dear Old Dad: Attempting to Check Balance\n");

            sem_wait(mutex);
            int localBalance = *BankAccount;

            int action = rand() % 2; // Even or Odd
            if (action == 0) { // Even -> try deposit if < 100
                if (localBalance < 100) {
                    int amount = rand() % 101; // 0 - 100
                    int deposit_chance = rand() % 2;
                    if (deposit_chance == 0) { // Even -> Deposit
                        localBalance += amount;
                        *BankAccount = localBalance;
                        printf("Dear old Dad: Deposits $%d / Balance = $%d\n", amount, localBalance);
                    } else { // Odd -> No money
                        printf("Dear old Dad: Doesn't have any money to give\n");
                    }
                } else {
                    printf("Dear old Dad: Thinks Student has enough Cash ($%d)\n", localBalance);
                }
            } else { // Odd -> Just check balance
                printf("Dear Old Dad: Last Checking Balance = $%d\n", localBalance);
            }
            sem_post(mutex);
        }
        exit(0);
    }

    // 4. Fork Lovable Mom (If enabled: arg[1] >= 2)
    if (mom_enabled >= 2) {
        pid_t mom_pid = fork();
        if (mom_pid < 0) {
            perror("fork mom failed");
            exit(1);
        } else if (mom_pid == 0) {
            // --- LOVABLE MOM PROCESS ---
            while (1) {
                sleep(rand() % 11); // Sleep 0-10 seconds
                printf("Loveable Mom: Attempting to Check Balance\n");

                sem_wait(mutex);
                int localBalance = *BankAccount;

                if (localBalance <= 100) {
                    int amount = rand() % 126; // 0 - 125
                    localBalance += amount;
                    *BankAccount = localBalance;
                    printf("Lovable Mom: Deposits $%d / Balance = $%d\n", amount, localBalance);
                }
                sem_post(mutex);
            }
            exit(0);
        }
    }

    // 5. Fork N Poor Student Children
    for (int i = 0; i < num_children; i++) {
        pid_t child_pid = fork();
        if (child_pid < 0) {
            perror("fork student failed");
            exit(1);
        } else if (child_pid == 0) {
            // --- POOR STUDENT PROCESS ---
            while (1) {
                sleep(rand() % 6); // Sleep 0-5 seconds
                printf("Poor Student: Attempting to Check Balance\n");

                sem_wait(mutex);
                int localBalance = *BankAccount;

                int action = rand() % 2; // Even or Odd
                if (action == 0) { // Even -> Withdraw money
                    int need = rand() % 51; // 0 - 50
                    printf("Poor Student needs $%d\n", need);
                    if (need <= localBalance) {
                        localBalance -= need;
                        *BankAccount = localBalance;
                        printf("Poor Student: Withdraws $%d / Balance = $%d\n", need, localBalance);
                    } else {
                        printf("Poor Student: Not Enough Cash ($%d)\n", localBalance);
                    }
                } else { // Odd -> Check last balance
                    printf("Poor Student: Last Checking Balance = $%d\n", localBalance);
                }
                sem_post(mutex);
            }
            exit(0);
        }
    }

    // Parent process waits for all children
    int total_processes = num_children + 1 + (mom_enabled >= 2 ? 1 : 0);
    for (int i = 0; i < total_processes; i++) {
        wait(NULL);
    }

    cleanup_and_exit(0);
    return 0;
}