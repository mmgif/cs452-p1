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

#define BUFFER_SIZE 1024
#define MAX_PATH_LENGTH 4096 

struct bgPid {
  int job;
  pid_t pid;
  char *cmd;
  bool done;
  bool seenDone;
};

int main(int argc, char * argv[]) {
  printf("hello world\n");

  const long ARG_MAX = sysconf(_SC_ARG_MAX);
  const char *JOBS = "jobs";

  struct shell sh;
  char *line;
  int bgPidsNum = 0;
  int bgPidsCapacity = 255;
  // pid_t bgPids[bgPidsSize];
  struct bgPid bgPids[bgPidsCapacity];
  int jobNum = 1;
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

      // fprintf(stderr, "are you what I think you are? %c\n", line[strlen(line) - 1]);
      if(line[strlen(line) - 1] == '&') {
        bg = true;
        // fprintf(stderr, "now you're in the background\n");
        // trim ampersand so command runs proper
        line[strlen(line) - 1] = '\0';
      }
      
      char **cmd = cmd_parse(line);

      // check for jobs here, so we can append the list of jobs to the cmd before it is decomposed ?
      // or as it is decomposed, remake it :pensive:
      if(strncmp(cmd[0], JOBS, strlen(JOBS) + 1) == 0 /*&& bgPidsNum != 0*/) {
        int padding = 20; // brackets, job num, DONE, spaces (11?)
        char tmpPid[12];

        char **newCmd = (char**) calloc(ARG_MAX, sizeof(char*));
        if(newCmd == NULL) {
          fprintf(stderr, "could not allocate array of strings\n");
        }
        newCmd[0] = (char*) malloc(sizeof(char) * (strlen(JOBS) + 1));
        if(newCmd[0] == NULL) {
          fprintf(stderr, "could not allocate string\n");
        }
        strncpy(newCmd[0], JOBS, strlen(JOBS) + 1);

        for(int ii = 0; ii < bgPidsNum; ii++) {
          int pidLen = snprintf(tmpPid, 12, "%d", bgPids[ii].pid);
         if(pidLen == -1) {
           fprintf(stderr, "could not get decimal representation of pid len\n");
         }
          int size = sizeof(char) * strlen(bgPids[ii].cmd) + pidLen + padding + 1;
          // int size = sizeof(char) * strlen(bgPids[ii].cmd) + 20 + padding + 1;

        //  char* buffer = (char*) malloc(size);
          newCmd[ii+1] = (char*) malloc(size);
          if(newCmd[ii+1] == NULL) {
            fprintf(stderr, "could not allocate string\n");
          }
          if(!bgPids[ii].seenDone) {
          if(bgPids[ii].done) {
        snprintf(newCmd[ii+1], size, "[%d] Done %s", bgPids[ii].job, bgPids[ii].cmd);

          } else {
        snprintf(newCmd[ii+1], size, "[%d] %d %s", bgPids[ii].job, bgPids[ii].pid, bgPids[ii].cmd);

          }
          // strncpy(tempLine[ii+1], )
        }
        
        }

        // assign templine to cmd, free old cmd
        char **oldCmd = cmd;
        cmd = newCmd;
        cmd_free(oldCmd);
      }


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
            // TODO do I also need to restore tmodes? I never take them out

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
              bgPids[bgPidsNum].done = false;
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

      int bgPidsDone = 0;      
      // go through history list and match??? on strings,,, maybe... unsure
      for(int ii = 0; ii < bgPidsNum; ii++) {

        if(bgPids[ii].done) {
          bgPidsDone++;
          if(!bgPids[ii].seenDone) {
            bgPids[ii].seenDone = true;
            fprintf(stdout, "[%d] Done %s\n", bgPids[ii].job, bgPids[ii].cmd);
            }
        }
      }
      // check if all jobs are done and seen done and reset job number to 1
        if(bgPidsDone == bgPidsNum) { // takes a second enter to reset,,,
          jobNum = 1;
          bgPidsNum = 0;  // overwrite beginning
        }
    }

    // actually, I want to free the children as soon as possible.
    // check every loop for done children...
    for(int ii = 0; ii < bgPidsNum; ii++) {
      if(!bgPids[ii].done) {
        int waitStatusBg;
        // free child from the <defunct> state if it is done
        pid_t waitBg = waitpid(bgPids[ii].pid, &waitStatusBg, WNOHANG);
        if(waitBg == -1) {
          perror("waitpid");
        }

        if(bgPids[ii].pid == waitBg) {  // waitpid() with WNOHANG returns pid when status changes
          bgPids[ii].done = true;
        }
      }
    }
    

   
    free(line);
  }

  fprintf(stderr, "free me");
  sh_destroy(&sh);
  return 0;
}
