#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>

#define MAX_LINE 1024
#define MAX_ARGS 64

extern char **environ;
pid_t foreground_pid = 0;

void handle_alarm(int sig) {
    if (foreground_pid > 0) {
        printf("\nProcess %d timed out (exceeded 10 seconds). Terminating...\n", foreground_pid);
        kill(foreground_pid, SIGKILL);
    }
}

void expand_env_vars(char **args) {
    for (int i = 0; args[i] != NULL; i++) {
        if (args[i][0] == '$') {
            char *val = getenv(args[i] + 1);
            args[i] = val ? val : "";
        }
    }
}

int handle_builtin(char **args) {
    if (args[0] == NULL) return 1;

    if (strcmp(args[0], "exit") == 0) {
        exit(0);
    } else if (strcmp(args[0], "cd") == 0) {
        char *dir = args[1] ? args[1] : getenv("HOME");
        if (chdir(dir) != 0) {
            perror("cd failed");
        }
        return 1;
    } else if (strcmp(args[0], "pwd") == 0) {
        char cwd[MAX_LINE];
        if (getcwd(cwd, sizeof(cwd))) {
            printf("%s\n", cwd);
        }
        return 1;
    } else if (strcmp(args[0], "echo") == 0) {
        for (int i = 1; args[i] != NULL; i++) {
            printf("%s%s", args[i], args[i+1] ? " " : "");
        }
        printf("\n");
        return 1;
    } else if (strcmp(args[0], "setenv") == 0) {
        if (args[1] != NULL) {
            char *eq = strchr(args[1], '=');
            if (eq) {
                *eq = '\0';
                setenv(args[1], eq + 1, 1);
            } else if (args[2] != NULL) {
                setenv(args[1], args[2], 1);
            }
        }
        return 1;
    } else if (strcmp(args[0], "env") == 0) {
        if (args[1] != NULL) {
            char *val = getenv(args[1]);
            if (val) printf("%s\n", val);
        } else {
            for (char **env = environ; *env != NULL; env++) {
                printf("%s\n", *env);
            }
        }
        return 1;
    }
    return 0;
}

// Executes a single command with potential I/O redirection (< and >)
void execute_single_command(char **args) {
    char *input_file = NULL;
    char *output_file = NULL;

    // Look for < or > operators
    for (int i = 0; args[i] != NULL; i++) {
        if (strcmp(args[i], "<") == 0) {
            input_file = args[i + 1];
            args[i] = NULL;
        } else if (strcmp(args[i], ">") == 0) {
            output_file = args[i + 1];
            args[i] = NULL;
        }
    }

    if (input_file) {
        int fd_in = open(input_file, O_RDONLY);
        if (fd_in < 0) {
            perror("Input redirection failed");
            exit(1);
        }
        dup2(fd_in, STDIN_FILENO);
        close(fd_in);
    }

    if (output_file) {
        int fd_out = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd_out < 0) {
            perror("Output redirection failed");
            exit(1);
        }
        dup2(fd_out, STDOUT_FILENO);
        close(fd_out);
    }

    if (execvp(args[0], args) < 0) {
        printf("execvp() failed: %s\nAn error occurred.\n", strerror(errno));
        exit(1);
    }
}

// Handles commands with pipes (|)
void execute_pipeline(char **args) {
    int pipe_pos = -1;
    for (int i = 0; args[i] != NULL; i++) {
        if (strcmp(args[i], "|") == 0) {
            pipe_pos = i;
            break;
        }
    }

    if (pipe_pos == -1) {
        // No pipe present
        execute_single_command(args);
        return;
    }

    // Split args into left and right commands
    args[pipe_pos] = NULL;
    char **left_args = args;
    char **right_args = &args[pipe_pos + 1];

    int pipefd[2];
    if (pipe(pipefd) < 0) {
        perror("pipe failed");
        exit(1);
    }

    pid_t p1 = fork();
    if (p1 == 0) {
        // Child 1 (Left side of pipe)
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[1]);
        execute_single_command(left_args);
    }

    pid_t p2 = fork();
    if (p2 == 0) {
        // Child 2 (Right side of pipe)
        close(pipefd[1]);
        dup2(pipefd[0], STDIN_FILENO);
        close(pipefd[0]);
        execute_single_command(right_args);
    }

    close(pipefd[0]);
    close(pipefd[1]);
    waitpid(p1, NULL, 0);
    waitpid(p2, NULL, 0);
    exit(0);
}

int main(int argc, char *argv[]) {
    char line[MAX_LINE];
    char *args[MAX_ARGS];

    signal(SIGINT, SIG_IGN);
    signal(SIGALRM, handle_alarm);

    while (1) {
        char cwd[MAX_LINE];
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            printf("%s>", cwd);
            fflush(stdout);
        }

        if (!fgets(line, sizeof(line), stdin)) break;

        line[strcspn(line, "\n")] = '\0';

        int i = 0;
        char *token = strtok(line, " \t");
        while (token != NULL && i < MAX_ARGS - 1) {
            args[i++] = token;
            token = strtok(NULL, " \t");
        }
        args[i] = NULL;

        if (args[0] == NULL) continue;

        expand_env_vars(args);

        if (handle_builtin(args)) continue;

        int is_background = 0;
        if (i > 0 && strcmp(args[i - 1], "&") == 0) {
            is_background = 1;
            args[i - 1] = NULL;
        }

        pid_t pid = fork();
        if (pid < 0) {
            perror("fork failed");
        } else if (pid == 0) {
            signal(SIGINT, SIG_DFL);
            execute_pipeline(args);
        } else {
            if (!is_background) {
                foreground_pid = pid;
                alarm(10);

                int status;
                waitpid(pid, &status, 0);

                alarm(0);
                foreground_pid = 0;
            } else {
                printf("[Process running in background, PID: %d]\n", pid);
            }
        }
    }

    return 0;
}