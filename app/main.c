/*
 * TODO doc comment
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

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
  //  returns the line read in a format suitable for execution
      if(!do_builtin(&sh, cmd)) {   // FIXY crying 'cuz it's null
        // printf("%s\n", line);

    // TODO if cmd ends in ampersand, then need to fork and not wait
    // otherwise, we should always fork and wait
        errno = 0;
        // int rVal = execvp(cmd[0], &cmd[1]);  // complains about char*const*
        int rVal = execvp(cmd[0], cmd);
        // FIXY may need to be cast to (char*const*)
        // any arguments are a bad address,,,,,
        // runs only the first command, no arguments
        if(rVal == -1) {
          perror("execvp");
        // TODO do soemthing, perhaps
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
