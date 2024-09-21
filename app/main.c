#include <stdio.h>

#include <readline/readline.h>
#include <readline/history.h>

#include "../src/lab.h"

int main(int argc, char * argv[])
{
  printf("hello world\n");

  char *line;

  parse_args(argc, argv);

  // initialize a new shell :)
  // sh_init();

  using_history();
  while((line = readline("$"))) {
    printf("%s\n", line);
    add_history(line);
    free(line);
  }

  // sh_destroy();

  return 0;
}
