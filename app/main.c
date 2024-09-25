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

int main(int argc, char * argv[])
{
  printf("hello world\n");

  struct shell sh;
  char *line;

  parse_args(argc, argv);
  sh_init(&sh);

  // setting these in sh init isn't going well, so they go here
      sh.shell_is_interactive = 0;
    sh.shell_pgid = 0;
    
    // set up the termios struct??
    // generated from onyx defaults
    sh.shell_tmodes.c_iflag = 0x6d02;
    sh.shell_tmodes.c_oflag = OPOST | ONLCR;   // 0x5
    sh.shell_tmodes.c_cflag = 0x4bf;
    sh.shell_tmodes.c_lflag = 0x8a3b;
    sh.shell_tmodes.c_line = 0x0;

    // there are only 17 defined control characters
    sh.shell_tmodes.c_cc[0] = VKILL;   // 0x3
    sh.shell_tmodes.c_cc[1] = 0x1c;
    sh.shell_tmodes.c_cc[2] = 0x7f;
    sh.shell_tmodes.c_cc[3] = 0x15;
    sh.shell_tmodes.c_cc[4] = VEOF;    // 0x4
    sh.shell_tmodes.c_cc[5] = VINTR;   // 0x0
    sh.shell_tmodes.c_cc[6] = VERASE;  // 0x1
    sh.shell_tmodes.c_cc[7] = VINTR;   // 0x0
    sh.shell_tmodes.c_cc[8] = 0x11;
    sh.shell_tmodes.c_cc[9] = 0x13;
    sh.shell_tmodes.c_cc[10] = 0x1a;
    sh.shell_tmodes.c_cc[11] = 0xff;
    sh.shell_tmodes.c_cc[12] = 0x12;
    sh.shell_tmodes.c_cc[13] = VLNEXT; // 0xf
    sh.shell_tmodes.c_cc[14] = 0x17;
    sh.shell_tmodes.c_cc[15] = 0x16;
    sh.shell_tmodes.c_cc[16] = 0xff;

    sh.shell_tmodes.c_ispeed = 0xf;
    sh.shell_tmodes.c_ospeed = 0xf;

    sh.shell_terminal = 0;
    sh.prompt = get_prompt("MY_PROMPT");
  // ---> ignore, probably

  // setup ps
  char *ps = (char*) malloc(sizeof(char) * (strlen(sh.prompt) + 2));
  snprintf(ps, sizeof(char) * (strlen(sh.prompt) + 2), "%s ", sh.prompt);
  
  using_history();
  while((line = readline(ps))) {
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