#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <readline/readline.h>
#include <readline/history.h>

#include "../src/lab.h"

int main(int argc, char * argv[])
{
  printf("hello world\n");

  struct shell sh;

  parse_args(argc, argv);
  sh_init(&sh);

  // char *ps = get_prompt("MY_PROMPT");
  // sh.prompt = ps;

  // char *line;


   // initialize a new shell :)
  // sh_init();

  // using_history();
  // while((line = readline("$ "))) {
  //   printf("%s\n", line);
    // replace above with call to parse line
    // char **command = cmd_parse(line);
    // returns the line read in a format suitable for execution

    // add_history(line);
    
    // cmd_free(line);
    // free the line from cmd_parse
    // free(line);
  // }

  // sh_destroy(&sh);
  // free(&sh);

  return 0;
}
