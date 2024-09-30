/* 
 * File: lab.c
 * Author: Emma Gifford
 * Date: Sun Sep 29 17:22:23 MDT 2024
 * Description: Functions for working with an interactive shell
 */

#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <pwd.h>
#include <errno.h>
#include <signal.h>
#include <readline/readline.h>
#include <readline/history.h>

#include "lab.h"

#define BUFFER_SIZE 1024
#define MAX_PATH_LENGTH 4096

char *get_prompt(const char *env){
  const char *FUNC_NAME = "get_prompt";
  char defaultPrompt[] = "shell>";
  char *prompt;
  char *envPtr;

  if (env != NULL) { // check for environment variable
    envPtr = getenv(env);
  } else { // use default environment variable
    envPtr = getenv("MY_PROMPT");
  }

  if (envPtr != NULL) {
    prompt = (char *)malloc(sizeof(char) * (strlen(envPtr) + 1));
    if (prompt == NULL)
    {
      fprintf(stderr, "%s: could not allocate string\n", FUNC_NAME);
      return NULL;
    }
    strncpy(prompt, envPtr, strlen(envPtr) + 1);
  } else { // use default prompt "shell>"
    prompt = (char *)malloc(sizeof(char) * (strlen(defaultPrompt) + 1));
    if (prompt == NULL) {
      fprintf(stderr, "%s: could not allocate string\n", FUNC_NAME);
      return NULL;
    }
    strncpy(prompt, defaultPrompt, strlen(defaultPrompt) + 1); // gotta copy it in there, silly
  }
  return prompt;
}

int change_dir(char **dir) {
  const char *FUNC_NAME = "change_dir";
  int pathDirNameSize;
  int dirNameSize;
  char *path;
  char *envPtr;
  int rVal;

  // detect if args has a directory listed
  if (dir[1] != NULL) {
    dirNameSize = sizeof(char) * (strlen(dir[1]) + 1);
    pathDirNameSize = dirNameSize;
    path = (char *)malloc(pathDirNameSize);
    if (path == NULL) {
      fprintf(stderr, "%s: could not allocate string\n", FUNC_NAME);
    }
    strncpy(path, dir[1], dirNameSize);
  } else {
    envPtr = getenv("HOME");
    if (envPtr == NULL) {
      uid_t uid = getuid();
      struct passwd *pwuid = getpwuid(uid);
      char *pwDir = pwuid->pw_dir;

      pathDirNameSize = sizeof(char) * (strlen(pwDir) + 1);
      path = (char *)malloc(pathDirNameSize);
      if (path == NULL) {
        fprintf(stderr, "%s: could not allocate string\n", FUNC_NAME);
      }
      strncpy(path, pwDir, pathDirNameSize);
    } else {
      pathDirNameSize = sizeof(char) * (strlen(envPtr) + 1);
      path = (char *)malloc(pathDirNameSize);
      if (path == NULL) {
        fprintf(stderr, "%s: could not allocate string\n", FUNC_NAME);
      }
      strncpy(path, envPtr, pathDirNameSize);
    }
  }

  errno = 0;
  rVal = chdir(path);
  if (rVal == -1) {
    perror("chdir");
  }

  free(path);
  return rVal;
}

char **cmd_parse(char const *line) {
  const char *FUNC_NAME = "cmd_parse";
  const long ARG_MAX = sysconf(_SC_ARG_MAX);
  char *tok;
  int ii;

  char **cmd = (char **)calloc(ARG_MAX, sizeof(char *)); // calloc to ensure everything else is initalized null
  if (cmd == NULL) {
    fprintf(stderr, "%s: could not allocate array of strings\n", FUNC_NAME);
  }
  char *lines = (char *)malloc(sizeof(char) * (strlen(line) + 1));
  if (lines == NULL) {
    fprintf(stderr, "%s: could not allocate string\n", FUNC_NAME);
  }
  strncpy(lines, line, strlen(line) + 1);

  // init the loop
  ii = 0;
  tok = strtok(lines, " "); // tokenize on spaces
  while (tok != NULL && ii < ARG_MAX) { // null returned at end of string
    cmd[ii] = (char *)malloc(sizeof(char) * (strlen(tok) + 1));
    if (cmd[ii] == NULL) {
      fprintf(stderr, "%s: could not allocate string\n", FUNC_NAME);
    }
    strncpy(cmd[ii], tok, strlen(tok) + 1);

    tok = strtok(NULL, " "); // scan where prev success call ended
    ii++;
  }
  free(lines);
  return cmd;
}

void cmd_free(char **line) {
  const long ARG_MAX = sysconf(_SC_ARG_MAX);
  int size = ARG_MAX;
  for (int ii = 0; ii < size; ii++) {
    if (line[ii] != NULL) {
      free(line[ii]);
    }
  }
  free(line);
}

char *trim_white(char *line) {
  const char *FUNC_NAME = "trim_white";
  int start;
  int end;
  int ii = 0;
  int jj = strlen(line) - 1; // not inclusive of \0, also needs zero indexing

  while (ii < (int)strlen(line) && (isspace(line[ii]) > 0)) {
    ii++;
  }
  start = ii;

  while (jj > 0 && (isspace(line[jj]) > 0)) {
    jj--;
  }
  end = jj + 1;

  int size = sizeof(char) * (abs(start - end) + 1);
  char *trimmed = malloc(size); // add one for \0
  if (trimmed == NULL) {
    fprintf(stderr, "%s: could not allocate string\n", FUNC_NAME);
  }

  jj = 0;
  for (ii = start; ii < end; ii++) {
    trimmed[jj] = line[ii]; // copy only valid parts of string
    jj++;
  }
  trimmed[jj] = '\0'; // add null terminator

  // copy trimmed into line, so that trimmed can be free'd
  strncpy(line, trimmed, size);
  free(trimmed);
  return line;
}

bool do_builtin(struct shell *sh, char **argv) {
  const char *FUNC_NAME = "do_builtin";
  const char *EXIT = "exit";
  const char *CD = "cd";
  const char *PWD = "pwd";
  const char *HISTORY = "history";
  const char *JOBS = "jobs";
  bool isBuiltIn = false;

  if (strncmp(argv[0], EXIT, strlen(EXIT) + 1) == 0) {
    // fprintf(stderr, "%s: exiting...\n", FUNC_NAME);
    isBuiltIn = true;
    sh_destroy(sh);
  }
  if (strncmp(argv[0], CD, strlen(CD) + 1) == 0) {
    // fprintf(stderr, "%s: changing dir...\n", FUNC_NAME);
    isBuiltIn = true;
    change_dir(argv);
  }
  if (strncmp(argv[0], PWD, strlen(PWD) + 1) == 0) {
    // fprintf(stderr, "%s: showing current working directory...", FUNC_NAME);
    isBuiltIn = true;
    errno = 0;
    char *buffer = getcwd(NULL, MAX_PATH_LENGTH);
    if(buffer == NULL) {
      perror("getcwd");
    }
    fprintf(stdout, "%s\n", buffer);
    free(buffer);
  }
  if (strncmp(argv[0], HISTORY, strlen(HISTORY) + 1) == 0) {
    // fprintf(stderr, "%s: showing hist...\n", FUNC_NAME);
    isBuiltIn = true;
    HIST_ENTRY **hist = history_list();
    if (hist == NULL) {
      // fprintf(stderr, "%s: no history\n", FUNC_NAME);
      fprintf(stderr, "%s: could not retrieve history\n", FUNC_NAME);
    } else {
      for (int ii = 0; ii < history_length; ii++) {
        fprintf(stdout, "  %-3d   %s\n", ii, hist[ii]->line);
      }
    }
  }
  if (strncmp(argv[0], JOBS, strlen(JOBS) + 1) == 0) {
    // fprintf(stderr, "%s: listing jobs...\n", FUNC_NAME);
    isBuiltIn = true;
    int size = sysconf(_SC_ARG_MAX);
    for (int ii = 1; ii < size; ii++) {
      if (argv[ii] != NULL) {
        fprintf(stdout, "%s", argv[ii]);
      }
    }
  }
  return isBuiltIn;
}

void sh_init(struct shell *sh) {
  const char *FUNC_NAME = "sh_init";
  int rVal;

  sh->shell_terminal = STDIN_FILENO;
  sh->shell_is_interactive = isatty(sh->shell_terminal);
  // fprintf(stderr, "%s: is shell interactive?: %d\n", FUNC_NAME, sh->shell_is_interactive);

  if (sh->shell_is_interactive) {
    // loop until in the foreground
    while (tcgetpgrp(sh->shell_terminal) != (sh->shell_pgid = getpgrp())) {
      kill(-sh->shell_pgid, SIGTTIN);
    }

    // Shell ignores these signals
    signal(SIGINT, SIG_IGN);
    signal(SIGQUIT, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
    signal(SIGTTOU, SIG_IGN);

    // put shell in own process group
    sh->shell_pgid = getpid();
    errno = 0;
    rVal = setpgid(sh->shell_pgid, sh->shell_pgid);
    if (rVal == -1) {
      perror("setpgid");
      fprintf(stderr, "%s: could not put shell in its own process group\n", FUNC_NAME);
      exit(1);
    }

    // use tcsetpgrp() to grab control of terminal
    errno = 0;
    rVal = tcsetpgrp(sh->shell_terminal, sh->shell_pgid);
    if (rVal == -1) {
      perror("tcsetpgrp");
    }

    // use tcgetattr() to save default terminal attributes for shell
    // set up the termios struct??
    errno = 0;
    rVal = tcgetattr(sh->shell_terminal, &(sh->shell_tmodes));
    if (rVal == -1) {
      perror("tcgetattr");
    }
  }

  char *prompt = get_prompt("MY_PROMPT");
  if (prompt == NULL) {
    fprintf(stderr, "%s: could not allocate string\n", FUNC_NAME);
  }
  sh->prompt = prompt;
}

void sh_destroy(struct shell *sh) {
  if (sh != NULL) {
    free(sh->prompt);
    exit(0);
  }
}

void parse_args(int argc, char **argv) {
  // Use getopt() to process command line arguments
  int opt = 0;
  while ((opt = getopt(argc, argv, "v")) != -1) {
    switch (opt) {
    case 'v': // print version
      fprintf(stdout, "version: %d.%d\n", lab_VERSION_MAJOR, lab_VERSION_MINOR);
      exit(0);
      break;
    default: // invalid arg
      // error handling: getopt prints its own errors for invalid option, so cute!
      exit(1);
      break;
    }
  }
}