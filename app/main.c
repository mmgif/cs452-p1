/*
 * TODO doc comment
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/wait.h>
#include <signal.h>

#include <readline/readline.h>
#include <readline/history.h>

#include "../src/lab.h"

// void initialize_readline(void);
// char *rl_gets(char *buffer, const char *prompt);


 /* Read a string, and return a pointer to it.
   Returns NULL on EOF. */
char *rl_gets(char *buffer, const char *prompt) {
  /* If the buffer has already been allocated,
     return the memory to the free pool. */
  if (buffer)
    {
      free (buffer);
      buffer = (char *)NULL;
    }

  /* Get a line from the user. */
  buffer = readline(prompt);

  /* If the line has any text in it,
     save it on the history. */
  if (buffer && *buffer)
    add_history (buffer);

  return (buffer);
} 


int main(int argc, char * argv[]) {
  printf("hello world\n");

  struct shell sh;
  char *line;

  parse_args(argc, argv);
  sh_init(&sh);

  // setup ps
  char ps[(sizeof(char) * (strlen(sh.prompt) + 2))];
  snprintf(ps, sizeof(char) * (strlen(sh.prompt) + 2), "%s ", sh.prompt);
  
  using_history();
  while((line = readline(ps))) {

    line = trim_white(line);
    
    if(line && *line) {   // goes before parsing command, so it shows in history
      add_history(line);
      
      char **cmd = cmd_parse(line);
      if(!do_builtin(&sh, cmd)) {
        // printf("%s\n", line);
    // TODO if cmd ends in ampersand, then need to fork and not wait
    // otherwise, we should always fork and wait

    // TODO need to do something with arg max, probably has to do with how big command parse is?,
    // or do I already check that in cmd parse? maybe
        pid_t cmdPid;
        pid_t w;
        int wStatus;

        cmdPid = fork();
        if(cmdPid == -1) {
          perror("fork");
          // FIXY error
        }

        if(cmdPid == 0) {  // this is inside the child process
          pid_t child = getpid();
          int rVal;

          // "initialize" child process
          errno = 0;
          rVal = setpgid(child, child);
          if(rVal == -1) {
            perror("setpgid");
          } 
          errno = 0;
          rVal = tcsetpgrp(sh.shell_terminal, child);
          if(rVal == -1) {
            perror("tcsetpgrp");
          }
          signal(SIGINT, SIG_DFL);          
          signal(SIGQUIT, SIG_DFL);
          signal(SIGTSTP, SIG_DFL);
          signal(SIGTTIN, SIG_DFL);
          signal(SIGTTOU, SIG_DFL);

          errno = 0;
          rVal = execvp(cmd[0], cmd);
          if(rVal == -1) {
            perror("execvp");
            sh_destroy(&sh);
            exit(rVal);
          }
        } else {  // this is the parent process
          do {
            w = waitpid(cmdPid, &wStatus, 0);  // more options for zero in the documentation
            if(w == -1) {
              perror("waitpid");
              // TODO EXIT?? or idk
            }
          // FIXY kinda works? but not what I want... have to press enter to move forward. stupid.
          // char c = getchar();
          // while(c != '\n' && c != EOF) {
            // c = getchar();
          // }
          } while(!WIFEXITED(wStatus) && !WIFSIGNALED(wStatus)); // FIXY do I need anything else?
          // FIXY need to exit the same amount of failed commands (spawning processes for failed excep commands?)
          // FIXY need to ignore typed commands when processes are waiting (are these the signals?)
          
        }
       
      }
    
      cmd_free(cmd);      
    }
   
    free(line);
  }

  fprintf(stderr, "free me"); // FIXY jumping to here after a command occurs (fail or not), something to do with line reading?

  sh_destroy(&sh);
  // free(ps);
  // free(sh.prompt);
  // free(&sh);

  return 0;
}

// void initialize_readline() {
  // rl_bind_key('\t', rl_insert);
// }
