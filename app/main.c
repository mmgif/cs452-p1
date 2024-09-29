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


struct bgPid {
  int job;
  pid_t pid;
  char *cmd;
  bool seenDone;
};

struct bgPid *expand_bgPids(struct bgPid *bgPids, int currLen);

int main(int argc, char * argv[]) {
  printf("hello world\n");

  struct shell sh;
  char *line;
  int bgPidsNum = 0;
  int bgPidsCapacity = 10;
  // pid_t bgPids[bgPidsSize];
  struct bgPid bgPids[bgPidsCapacity];
  int jobNum = 0;
  bool bg;

  parse_args(argc, argv);
  sh_init(&sh);

  // setup ps
  char ps[(sizeof(char) * (strlen(sh.prompt) + 2))];
  snprintf(ps, sizeof(char) * (strlen(sh.prompt) + 2), "%s ", sh.prompt);
  
  using_history();
  while((line = readline(ps))) {

    line = trim_white(line);
    bg = false;
    
    if(line && *line) {   // goes before parsing command, so it shows in history
      add_history(line);

    // TODO if cmd ends in ampersand, then need to fork and not wait
    // otherwise, we should always fork and wait

      // fprintf(stderr, "are you what I think you are? %c\n", line[strlen(line) - 1]);
      if(line[strlen(line) - 1] == '&') {
        bg = true;
        // fprintf(stderr, "now you're in the background\n");
        // trim ampersand so command runs proper
        line[strlen(line) - 1] = '\0';
      }
      
      char **cmd = cmd_parse(line);
      if(!do_builtin(&sh, cmd)) {
        // printf("%s\n", line);
        pid_t cmdPid;
        pid_t wait;
        int waitStatus;

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

          if(!bg) {
            errno = 0;
            rVal = tcsetpgrp(sh.shell_terminal, child);
            if(rVal == -1) {
              perror("tcsetpgrp");
            }
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

        } else if (cmdPid > 0) {  // this is the parent process
          int rVal;
          // need to set pgid from parent of child?
          errno = 0;
          rVal = setpgid(cmdPid, cmdPid);
          if(rVal == -1) {
            perror("setpgid");
          }

          // need to put child into foreground with tcsetgrp
          if(!bg) {
            errno = 0;
            rVal = tcsetpgrp(sh.shell_terminal, cmdPid);
            if(rVal == -1) {
              perror("tcsetpgrp");
            }
        
          do {
            wait = waitpid(cmdPid, &waitStatus, 0);  // more options for zero in the documentation
            if(wait == -1) {
              perror("waitpid");
              // TODO EXIT?? or idk
            }
          } while(!WIFEXITED(waitStatus) && !WIFSIGNALED(waitStatus));

          // need to get back the process group? yes!
             errno = 0;
            rVal = tcsetpgrp(sh.shell_terminal, sh.shell_pgid);
            if(rVal == -1) {
              perror("tcsetpgrp");
            }

            // flush the input, do not remember anything typed while command was executing
            tcflush(sh.shell_terminal, TCIOFLUSH);
          } else {
              // TODO we do something here to handle background processes
              if(bgPidsNum >= bgPidsCapacity) {
                // bgPids = expand_bgPids(bgPids, bgPidsCapacity);
                fprintf(stderr, "expand bgpids pls\n");
              }

              bgPids[bgPidsNum].job = jobNum;
              bgPids[bgPidsNum].pid = cmdPid;
              bgPids[bgPidsNum].seenDone = false;

              HIST_ENTRY *bgCmd = current_history();
              if(bgCmd == NULL) {
                fprintf(stderr, "uh oh, the background command escaped\n");
              }

              bgPids[bgPidsNum].cmd = bgCmd->line;

              fprintf(stdout, "[%d] %d %s\n", bgPids[bgPidsNum].job, bgPids[bgPidsNum].pid, bgPids[bgPidsNum].cmd);

              bgPidsNum++;
              jobNum++;

          }
        }
      }
    
      cmd_free(cmd);      
    } else {
      fprintf(stderr, "print me\n");
      // TODO print info about background processes here
      // this is for the last time press enter key stuff
      
      // go through history list and match??? on strings,,, maybe... unsure
      for(int ii = 0; ii < bgPidsNum; ii++) {

        if(!bgPids[ii].seenDone) {
          int waitStatusBg;
          waitpid(bgPids[ii].pid, &waitStatusBg, WNOHANG);

          if(WIFEXITED(waitStatusBg) /*|| WIFSIGNALED(waitStatusBg) */) {
            bgPids[ii].seenDone = true;
            fprintf(stdout, "[%d] Done %s\n", bgPids[ii].job, bgPids[ii].cmd);
          }
        }
      }

    }
   
    free(line);
  }

  fprintf(stderr, "free me"); // FIXY jumping to here after a command occurs (fail or not), something to do with line reading?
  sh_destroy(&sh);
  return 0;
}

struct bgPid *expand_bgPids(struct bgPid *bgPids, int currLen) {
  UNUSED(currLen);
  fprintf(stderr, "expand_bgPids not implemented");
  return bgPids;
}