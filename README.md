# Project 1: Quash (Quite Another Shell)

**Author(s):** Geehan Altayb
**Course:** Operating Systems  
**Date:** October 1, 2026  
**Repository:** https://github.com/latartademugello5/OS_FALL2026

---

## 1. Overview & Architecture

Quash is a custom command-line interpreter (shell) written in C. It demonstrates core operating system concepts including process creation, process execution, environment variable handling, signal manipulation, time-limited process management, file I/O redirection, and inter-process communication using pipes.

### Key Features Implemented
- **Interactive Prompt:** Displays the current working directory (`<cwd>> `) using `getcwd()`.
- **Built-in Commands:** Native handling of `cd`, `pwd`, `echo`, `env`, `setenv`, and `exit`.
- **Variable Expansion:** Automatically expands variables prefixed with `$` (e.g., `$HOME`, `$VAR`) across all commands using `getenv()`.
- **Process Forking & Execution:** Executes external system binaries using `fork()`, `execvp()`, and `waitpid()`.
- **Background Execution:** Appending `&` to a command executes it asynchronously without blocking the parent shell.
- **Signal Handling:** Ignores `SIGINT` (Ctrl+C) in the interactive shell prompt, ensuring the shell does not terminate when interrupting child processes.
- **Timeout Management:** Terminates foreground processes exceeding a 10-second limit using `SIGALRM` and `kill()`.
- **I/O Redirection:** Directs input using `<` and output using `>` via low-level file descriptors (`open`, `dup2`).
- **Inter-Process Communication (Pipes):** Supports piping standard output of one command to the standard input of another (`cmd1 | cmd2`) using `pipe()` and `dup2()`.

---

## 2. Design Choices & Implementation Details

### A. Command Parsing & Tokenization
The main control loop uses `fgets()` to read full command lines from standard input. Input strings are tokenized using `strtok()` with whitespace and tab delimiters. Before execution, the argument array is scanned for tokens starting with `$`. The function `expand_env_vars()` resolves these tokens against the process environment using `getenv()`.

### B. Built-in Commands vs. External Execution
Before spawning child processes, the shell checks if the primary command matches a built-in handler (`handle_builtin()`):
- **`cd`**: Updates the process's working directory via `chdir()`. Defaults to `HOME` if no path argument is provided.
- **`pwd`**: Retrieves and prints the current directory.
- **`echo`**: Iterates through arguments and prints them to stdout, utilizing pre-expanded environment variables.
- **`setenv`**: Supports both `setenv VAR VALUE` and `setenv VAR=VALUE` formats by invoking `setenv()`.
- **`env`**: Prints all active environment variables via `extern char **environ`, or queries a specific variable name.
- **`exit`**: Gracefully terminates the shell process with `exit(0)`.

If the command is not a built-in, control flow shifts to external process management.

### C. Process Creation & Background Jobs
When executing external binaries:
1. The shell checks if the last token is `&`. If present, it sets an `is_background` flag and removes `&` from the argument list.
2. A new process is created using `fork()`.
3. The parent process tracks background execution:
   - **Foreground:** Parent sets a 10-second alarm via `alarm(10)` and waits for the child using `waitpid()`.
   - **Background:** Parent prints the child PID and immediately loops back for the next user input without waiting.

### D. Signal Handling & Timeouts
To ensure robust shell lifetime:
- `signal(SIGINT, SIG_IGN)` is set in the parent shell so pressing Ctrl+C does not kill Quash. In child processes, default signal behavior is restored with `signal(SIGINT, SIG_DFL)`.
- `signal(SIGALRM, handle_alarm)` catches timeout alarms. If a foreground process exceeds 10 seconds, `handle_alarm()` sends a `SIGKILL` signal to `foreground_pid`.

### E. I/O Redirection & Piping
Redirection and piping are implemented inside child processes to ensure the main shell environment remains unpolluted:
- **Redirection (`<`, `>`):** The `execute_single_command()` function scans arguments for `<` or `>`. When found, `open()` opens the target file descriptor and `dup2()` overwrites `STDIN_FILENO` or `STDOUT_FILENO`.
- **Piping (`|`):** The `execute_pipeline()` function locates `|`, splits the argument list into two sub-commands, creates a unidirectional channel using `pipe()`, and forks two child processes. The left process redirects stdout to `pipefd[1]`, while the right process redirects stdin from `pipefd[0]`.

---

## 3. Code Documentation & Function Breakdown

| Function | Signature | Description |
| :--- | :--- | :--- |
| `main` | `int main(int argc, char *argv[])` | Main shell loop. Handles signal setup, prompt display, input tokenization, background detection, and child process management. |
| `handle_builtin` | `int handle_builtin(char **args)` | Checks if `args[0]` matches a built-in command and executes it inline. Returns `1` if handled, `0` otherwise. |
| `expand_env_vars` | `void expand_env_vars(char **args)` | Scans argument tokens for leading `$` characters and replaces them with environment variable values. |
| `execute_single_command` | `void execute_single_command(char **args)` | Handles `<` and `>` file redirection setup via `dup2()`, then calls `execvp()`. |
| `execute_pipeline` | `void execute_pipeline(char **args)` | Checks for `|` tokens. If present, creates a pipe and two child processes to connect stdout and stdin across commands. |
| `handle_alarm` | `void handle_alarm(int sig)` | Signal handler for `SIGALRM`. Sends `SIGKILL` to foreground processes that exceed the 10-second limit. |

---

## 4. Compilation & Usage

### Building the Project
A `Makefile` is provided to compile the shell source code:
```bash
make
