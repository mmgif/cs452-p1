/*
 * TODO doc comment
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
  char *ps = (char*) malloc(sizeof(char) * (strlen(sh.prompt) + 2));
  if(ps == NULL) {
    fprintf(stderr, "could not allocate string\n");
  }
  snprintf(ps, sizeof(char) * (strlen(sh.prompt) + 2), "%s ", sh.prompt);
  
  using_history();
  while((line = readline(ps))) {  // NOTE this should probably be fine, actually, since I fixed the exiting issues...
    // if readline encounters EOF, line is empty at point and (char*) NULL is returned
    // FIXY does not respond to ctrl+d (EOF)
    
    if(line && *line) {   // goes before parsing command, so it shows in history
      add_history(line);
    }

    // printf("%s\n", line);
//    replace above with call to parse line
   char **cmd = cmd_parse(line);
  //  returns the line read in a format suitable for execution
    if(!do_builtin(&sh, cmd)) {
      // TODO execvp system call 
    }
    
    cmd_free(cmd);
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
