/*
 * File: main.c
 * Author: Emma Gifford
 * Date: Sun Sep 29 17:22:23 MDT 2024
 * Description: Runs an interactive simple shell program
 *    which can start background processes
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

int main(int argc, char *argv[]) {
  printf("hello world\n");

  const long ARG_MAX = sysconf(_SC_ARG_MAX);
  const char *JOBS = "jobs";

  struct shell sh;
  char *line;
  int bgPidsNum = 0;
  int bgPidsCapacity = 255;
  struct bgPid bgPids[bgPidsCapacity];
  int jobNum = 1;
  bool bg;

  parse_args(argc, argv);
  sh_init(&sh);

  // setup ps
  char ps[(sizeof(char) * (strlen(sh.prompt) + 2))];
  snprintf(ps, sizeof(char) * (strlen(sh.prompt) + 2), "%s ", sh.prompt);

  using_history();
  while ((line = readline(ps))) {
    line = trim_white(line);
    bg = false;

    if (line && *line) {
      add_history(line);  // goes before parsing command, so it shows in history

      if (line[strlen(line) - 1] == '&') {
        bg = true;
        line[strlen(line) - 1] = '\0';  // trim ampersand so command runs proper
      }

      char **cmd = cmd_parse(line);

      // if the cmd is "jobs", append jobs display to the cmd before passing to do_builtin
      if (strncmp(cmd[0], JOBS, strlen(JOBS) + 1) == 0) {
        int padding = 50; // brackets, job num, DONE, RUNNING, spaces
        char tmpPid[12];  // anywhere between 2 and 12
        int bgPidsDone = 0;

        char **newCmd = (char **)calloc(ARG_MAX, sizeof(char *));
        if (newCmd == NULL) {
          fprintf(stderr, "could not allocate array of strings\n");
        }

        newCmd[0] = (char *)malloc(sizeof(char) * (strlen(JOBS) + 1));
        if (newCmd[0] == NULL) {
          fprintf(stderr, "could not allocate string\n");
        }
        strncpy(newCmd[0], JOBS, strlen(JOBS) + 1);

        for (int ii = 0; ii < bgPidsNum; ii++) {
          int pidLen = snprintf(tmpPid, 12, "%d", bgPids[ii].pid);
          if (pidLen == -1) {
            fprintf(stderr, "could not get decimal representation length of pid\n");
            pidLen = 12;
          }
          int size = sizeof(char) * (strlen(bgPids[ii].cmd) + pidLen + padding + 1);

          newCmd[ii + 1] = (char *)calloc(size / sizeof(char), sizeof(char));
          if (newCmd[ii + 1] == NULL) {
            fprintf(stderr, "could not allocate string\n");
          }
          if (!bgPids[ii].seenDone) { // append to the list only if the process is running, or has not been seen done
            if (bgPids[ii].done) {
              snprintf(newCmd[ii + 1], size, "[%d] Done %s\n", bgPids[ii].job, bgPids[ii].cmd);
              bgPids[ii].seenDone = true;
              bgPidsDone++;
            } else {
              snprintf(newCmd[ii + 1], size, "[%d] %d Running %s\n", bgPids[ii].job, bgPids[ii].pid, bgPids[ii].cmd);
            }
          }
        }
        // check if all jobs are done and seen done and reset job number to 1
        if (bgPidsDone == bgPidsNum) {
          jobNum = 1;
          bgPidsNum = 0; // overwrite at beginning
        }

        char **oldCmd = cmd;
        cmd = newCmd;
        cmd_free(oldCmd);
      }

      if (!do_builtin(&sh, cmd)) {
        pid_t cmdPid;
        pid_t wait;
        int waitStatus;

        cmdPid = fork();
        if (cmdPid == -1) {
          perror("fork");
        }

        if (cmdPid == 0) { // this is inside the child process
          pid_t child = getpid();
          int rVal;

          // "initialize" child process
          errno = 0;
          rVal = setpgid(child, child);
          if (rVal == -1) {
            perror("setpgid");
          }

          if (!bg) {
            errno = 0;
            rVal = tcsetpgrp(sh.shell_terminal, child);
            if (rVal == -1) {
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
          if (rVal == -1) {
            perror("execvp");
            sh_destroy(&sh);
            exit(rVal);
          }
        }
        else if (cmdPid > 0) { // this is the parent process
          int rVal;
          // set pgid from parent of child
          errno = 0;
          rVal = setpgid(cmdPid, cmdPid);
          if (rVal == -1) {
            perror("setpgid");
          }

          if (!bg) {  // put child into foreground with tcsetgrp
            errno = 0;
            rVal = tcsetpgrp(sh.shell_terminal, cmdPid);
            if (rVal == -1) {
              perror("tcsetpgrp");
            }

            do {  // wait for process to finish
              wait = waitpid(cmdPid, &waitStatus, 0);
              if (wait == -1) {
                perror("waitpid");
              }
            } while (!WIFEXITED(waitStatus) && !WIFSIGNALED(waitStatus));

            // return shell to foreground
            errno = 0;
            rVal = tcsetpgrp(sh.shell_terminal, sh.shell_pgid);
            if (rVal == -1) {
              perror("tcsetpgrp");
            }

            // flush the input, do not remember anything typed while command was executing
            tcflush(sh.shell_terminal, TCIOFLUSH);
          } else {
            // handle a process being set as a background process
            if (bgPidsNum >= bgPidsCapacity) {
              fprintf(stderr, "maximum number of background processes reached\n");
            }

            bgPids[bgPidsNum].job = jobNum;
            bgPids[bgPidsNum].pid = cmdPid;
            bgPids[bgPidsNum].done = false;
            bgPids[bgPidsNum].seenDone = false;

            HIST_ENTRY *bgCmd = current_history();
            if (bgCmd == NULL) {
              fprintf(stderr, "could not retrieve command for background process\n");
            }
            bgPids[bgPidsNum].cmd = bgCmd->line;

            fprintf(stdout, "[%d] %d %s\n", bgPids[bgPidsNum].job, bgPids[bgPidsNum].pid, bgPids[bgPidsNum].cmd);

            bgPidsNum++;
            jobNum++;
          }
        }
      }
      cmd_free(cmd);
    } else {  // print info about background processes finishing if user presses enter
      int bgPidsDone = 0;
      for (int ii = 0; ii < bgPidsNum; ii++) {
        if (bgPids[ii].done) {
          if (!bgPids[ii].seenDone) {
            bgPids[ii].seenDone = true;
            fprintf(stdout, "[%d] Done %s\n", bgPids[ii].job, bgPids[ii].cmd);
            bgPidsDone++;
          } else {
            bgPidsDone++;
          }
        }
      }
      // check if all jobs are done and seen done and reset job number to 1
      if (bgPidsDone == bgPidsNum) {
        jobNum = 1;
        bgPidsNum = 0; // overwrite at beginning
      }
    }

    // free children as soon as possible, prevent <defunct> processes
    // check every loop for done children...
    for (int ii = 0; ii < bgPidsNum; ii++) {
      if (!bgPids[ii].done) {
        int waitStatusBg;
        // free child from the <defunct> state if it is done
        pid_t waitBg = waitpid(bgPids[ii].pid, &waitStatusBg, WNOHANG);
        if (waitBg == -1) {
          perror("waitpid");
        }

        if (bgPids[ii].pid == waitBg) { // waitpid() with WNOHANG returns pid when status changes
          bgPids[ii].done = true;
        }
      }
    }
    free(line);
  }

  sh_destroy(&sh);
  return 0;
}
