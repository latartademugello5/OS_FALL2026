#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

int main(int argc, char **argv)
{
  // Check if search argument is provided
  if (argc < 2) {
      fprintf(stderr, "Usage: %s <search_term>\n", argv[0]);
      return 1;
  }

  int pipe1[2]; // Pipe between P1 (cat) and P2 (grep)
  int pipe2[2]; // Pipe between P2 (grep) and P3 (sort)

  char *cat_args[] = {"cat", "scores", NULL};
  char *grep_args[] = {"grep", argv[1], NULL};
  char *sort_args[] = {"sort", NULL};

  // Create both pipes
  if (pipe(pipe1) == -1 || pipe(pipe2) == -1) {
      perror("pipe");
      return 1;
  }

  int pid1 = fork();

  if (pid1 == 0) {
      // Child Process (P2 or P3)
      int pid2 = fork();

      if (pid2 == 0) {
          // Grandchild Process (P3): Executes 'sort'
          // Standard input comes from pipe2 reading end
          dup2(pipe2[0], 0);

          // Close all unused pipe file descriptors
          close(pipe1[0]);
          close(pipe1[1]);
          close(pipe2[0]);
          close(pipe2[1]);

          execvp("sort", sort_args);
          perror("execvp sort");
          exit(1);
      } else {
          // Child Process (P2): Executes 'grep <arg>'
          // Standard input comes from pipe1 reading end
          dup2(pipe1[0], 0);
          // Standard output goes to pipe2 writing end
          dup2(pipe2[1], 1);

          // Close all unused pipe file descriptors
          close(pipe1[0]);
          close(pipe1[1]);
          close(pipe2[0]);
          close(pipe2[1]);

          execvp("grep", grep_args);
          perror("execvp grep");
          exit(1);
      }
  } else {
      // Parent Process (P1): Executes 'cat scores'
      // Standard output goes to pipe1 writing end
      dup2(pipe1[1], 1);

      // Close all unused pipe file descriptors
      close(pipe1[0]);
      close(pipe1[1]);
      close(pipe2[0]);
      close(pipe2[1]);

      execvp("cat", cat_args);
      perror("execvp cat");
      exit(1);
  }

  return 0;
}