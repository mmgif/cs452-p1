/*
 * TODO doc comment
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/wait.h>

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

  // setup ps   // TODO may change this? is it leaking?
  char *ps = (char*) malloc(sizeof(char) * (strlen(sh.prompt) + 2));
  if(ps == NULL) {
    fprintf(stderr, "could not allocate string\n");
  }
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
        pid_t cmd_pid;
        pid_t w;
        int wstatus;

        cmd_pid = fork();
        if(cmd_pid == -1) {
          perror("fork");
          // TODO error
        }

        if(cmd_pid == 0) {
          errno = 0;
          int rVal = execvp(cmd[0], cmd);
          if(rVal == -1) {
            perror("execvp");
            sh_destroy(&sh);
            exit(rVal);
          }
        } else {
          do {
            w = waitpid(cmd_pid, &wstatus, 0);  // more options for zero in thhe documentation
            if(w == -1) {
              perror("waitpid");
              // TODO EXIT?? or idk
            }
          } while(!WIFEXITED(wstatus)); // FIXY do I need anything else?
          // FIXY need to exit the same amount of failed commands (spawning processes for failed excep commands?)
          // FIXY need to ignore typed commands when processes are waiting (are these the signals?)
        }
       
      }
    
      cmd_free(cmd);      
    }
   
    free(line);
  }

  sh_destroy(&sh);
  free(ps);
  // free(sh.prompt);
  // free(&sh);

  return 0;
}

// void initialize_readline() {
  // rl_bind_key('\t', rl_insert);
// }
