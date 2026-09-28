// C program to demonstrate use of fork() and pipe() 
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <sys/wait.h>

int main()
{
    // fd1: P1 -> P2
    // fd2: P2 -> P1
    int fd1[2]; 
    int fd2[2]; 

    char fixed_str1[] = "howard.edu";
    char fixed_str2[] = "gobison.org";

    char input_str[100];
    char input_str2[100];
    char concat_str[200];

    pid_t p;

    if (pipe(fd1) == -1) {
        fprintf(stderr, "Pipe 1 Failed\n");
        return 1;
    }
    if (pipe(fd2) == -1) {
        fprintf(stderr, "Pipe 2 Failed\n");
        return 1;
    }

    // Prompt P1 first input
    printf("Input : ");
    if (scanf("%s", input_str) == EOF) return 0;

    p = fork();

    if (p < 0) {
        fprintf(stderr, "fork Failed\n");
        return 1;
    }

    // Parent Process (P1)
    else if (p > 0) {
        // Close unused ends in Parent
        close(fd1[0]); // Close read end of pipe 1
        close(fd2[1]); // Close write end of pipe 2

        // Send string to child P2
        write(fd1[1], input_str, strlen(input_str) + 1);
        close(fd1[1]); // Done writing to P2

        // Wait for child process P2 to finish reading input and writing to fd2
        wait(NULL);

        // Read modified string back from child P2
        read(fd2[0], concat_str, sizeof(concat_str));
        close(fd2[0]); // Done reading from P2

        // Concatenate "gobison.org" and output
        strcat(concat_str, fixed_str2);
        printf("Output : %s\n", concat_str);
    }

    // Child Process (P2)
    else {
        // Close unused ends in Child
        close(fd1[1]); // Close write end of pipe 1
        close(fd2[0]); // Close read end of pipe 2

        // Read string from Parent P1
        read(fd1[0], concat_str, sizeof(concat_str));
        close(fd1[0]); // Done reading from P1

        // Concatenate "howard.edu" and print to stdout
        strcat(concat_str, fixed_str1);
        printf("Output : %s\n", concat_str);

        // Prompt user for second input
        printf("Input : ");
        scanf("%s", input_str2);

        // Append second input
        strcat(concat_str, input_str2);

        // Pass updated string back to Parent P1 via fd2
        write(fd2[1], concat_str, strlen(concat_str) + 1);
        close(fd2[1]); // Done writing to P1

        exit(0);
    }

    return 0;
}