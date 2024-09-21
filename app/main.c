#include <stdio.h>

#include <readline/readline.h>
#include <readline/history.h>

#include "../src/lab.h"

int main(int argc, char * argv[])
{
  printf("hello world\n");

  char *line;

  parse_args(argc, argv);

  char *prompt = get_prompt("MY_PROMPT");

  printf("prompt: \"%s\"", prompt);


  // initialize a new shell :)
  // sh_init();

  using_history();
  while((line = readline("$ "))) {
    printf("%s\n", line);
    // replace above with call to parse line
    // char **command = cmd_parse(line);
    // returns the line read in a format suitable for execution

    add_history(line);
    
    // cmd_free(line);
    // free the line from cmd_parse
    free(line);
  }

  // sh_destroy();

  return 0;
}
