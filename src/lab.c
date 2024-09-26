/* TODO doc comment
 *
 */

#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <pwd.h>
#include <errno.h>
#include <readline/readline.h>
#include <readline/history.h>

#include "lab.h"

#define MAX_FILEPATH_SIZE 4096 
//   struct shell
//   {
//     int shell_is_interactive;
//     pid_t shell_pgid;
//     struct termios shell_tmodes;
//     int shell_terminal;
//     char *prompt;
//   };



  /*
   * @brief Set the shell prompt. This function will attempt to load a prompt
   * from the requested environment variable, if the environment variable is
   * not set a default prompt of "shell>" is returned.  This function calls
   * malloc internally and the caller must free the resulting string.
   *
   * @param env The environment variable
   * @return const char* The prompt
   */
  char *get_prompt(const char *env) {
    const char *FUNC_NAME = "get_prompt";
    char defaultPrompt[] = "shell>";
    char *prompt;
    char *envPtr;

    if(env != NULL) { // check for environment variable
        envPtr = getenv(env);
    } else {          // use default environment variable
        envPtr = getenv("MY_PROMPT");
    }

    if(envPtr != NULL) {
      prompt = (char*)malloc(sizeof(char) * (strlen(envPtr) + 1));
      if(prompt == NULL) {
        fprintf(stderr, "%s: could not allocate string\n", FUNC_NAME);
        return NULL;
      }
      // fprintf(stderr, "%s: allocated string\n", METHOD_NAME);
      strncpy(prompt, envPtr, strlen(envPtr) + 1);
    } else {    // use default prompt "shell>" 
      prompt = (char*)malloc(sizeof(char) * (strlen(defaultPrompt) + 1));
      if(prompt == NULL) {
          fprintf(stderr, "%s: could not allocate string\n", FUNC_NAME);
          return NULL;
        }
        // fprintf(stderr, "%s: allocated string\n", METHOD_NAME);
        strncpy(prompt, defaultPrompt, strlen(defaultPrompt) + 1);  // gotta copy it in there, silly
        // fprintf(stderr, "%s: prompt: %s\n", METHOD_NAME, prompt);
    }
    return prompt;
  }

  /*
   * Changes the current working directory of the shell. Uses the linux system
   * call chdir. With no arguments the users home directory is used as the
   * directory to change to.
   *
   * @param dir The directory to change to
   * @return  On success, zero is returned.  On error, -1 is returned, and
   * errno is set to indicate the error.
   */
  int change_dir(char **dir) {
    const char *FUNC_NAME = "change_dir";
    int pathDirNameSize;
    int dirNameSize;
    char *path;
    char *envPtr;
    int rVal;

    // detect if args has a directory listed
    if(dir[1] != NULL) {
      dirNameSize = sizeof(char) * (strlen(dir[1]) + 1);
      pathDirNameSize = dirNameSize;
      path = (char*) malloc(pathDirNameSize);
      if(path == NULL) {
       fprintf(stderr, "%s: could not allocate string\n", FUNC_NAME);
      }
      strncpy(path, dir[1], dirNameSize);
    } else {
      envPtr = getenv("HOME");
      if(envPtr == NULL) {
        uid_t uid = getuid();
        struct passwd *pwuid = getpwuid(uid);
        char* pwDir = pwuid->pw_dir;

        pathDirNameSize = sizeof(char) * (strlen(pwDir) + 1);
        path = (char*) malloc(pathDirNameSize);
        if(path == NULL) {
          fprintf(stderr, "%s: could not allocate string\n", FUNC_NAME);
        }
        strncpy(path, pwDir, pathDirNameSize);
      } else {
        pathDirNameSize = sizeof(char) * (strlen(envPtr) + 1);
        path = (char*) malloc(pathDirNameSize);
        if(path == NULL) {
          fprintf(stderr, "%s: could not allocate string\n", FUNC_NAME);
        }
        strncpy(path, envPtr, pathDirNameSize);
      }
    }
    // fprintf(stderr, "%s: full path: %s\n", METHOD_NAME, path);
    errno = 0;
    rVal = chdir(path);
    if(rVal == -1) {
      perror("chdir");  // print error from errno
    }

    free(path);
    return rVal;
  }

  /*
   * @brief Convert line read from the user into to format that will work with
   * execvp. We limit the number of arguments to ARG_MAX loaded from sysconf.
   * This function allocates memory that must be reclaimed with the cmd_free
   * function.
   *
   * @param line The line to process
   *
   * @return The line read in a format suitable for exec
   */
  char **cmd_parse(char const *line) {
    const char *FUNC_NAME = "cmd_parse";
    const long ARG_MAX = sysconf(_SC_ARG_MAX);
    char *tok;
    int ii = 0;

    char **cmd = (char**) calloc(ARG_MAX, sizeof(char*));   // calloc to ensure everything else is initalized null
    if(cmd == NULL) {
      fprintf(stderr, "%s: could not allocate strings\n", FUNC_NAME);
    }
        char * lines = (char*) malloc(sizeof(char) * (strlen(line) + 1));
    if(lines == NULL) {
      fprintf(stderr, "%s: could not allocate string\n", FUNC_NAME);
    }
    strncpy(lines, line, strlen(line) + 1);

    // init the loop
    tok = strtok(lines, " ");   // tokenize on spaces
    while(tok != NULL) {        // null returned at end of string
      cmd[ii] = (char*) malloc(sizeof(char) * (strlen(tok) + 1));
      if(cmd[ii] == NULL) {
        fprintf(stderr, "%s: could not allocate string\n", FUNC_NAME);
      }
      strncpy(cmd[ii], tok, strlen(tok) + 1);

      tok = strtok(NULL, " ");  // scan where prev success call ended
      ii++;
    }
    free(lines);
    return cmd;
  }

  /*
   * @brief Free the line that was constructed with parse_cmd
   *
   * @param line the line to free
   */
  void cmd_free(char ** line) {
    const long ARG_MAX = sysconf(_SC_ARG_MAX);
    int size = ARG_MAX;
    for(int ii = 0; ii < size; ii++) {
      if(line[ii] != NULL) {
        free(line[ii]);
      }
    } 
    free(line);
  }

  /*
   * @brief Trim the whitespace from the start and end of a string.
   * For example "   ls -a   " becomes "ls -a". This function modifies
   * the argument line so that all printable chars are moved to the
   * front of the string
   *
   * @param line The line to trim
   * @return The new line with no whitespace
   */
  char *trim_white(char *line) {
    const char *FUNC_NAME = "trim_white";
    int start;
    int end;
    int ii = 0;
    int jj = strlen(line) - 1;  // not inclusive of \0, also needs zero indexing

    while(ii < (int)strlen(line) && (isspace(line[ii]) > 0)) {
      ii++;
    }
    start = ii;

    while(jj > 0 && (isspace(line[jj]) > 0)) {
      jj--;
    }
    end = jj + 1;

    int size = sizeof(char) * (abs(start - end) + 1);
    char *trimmed = malloc(size);  // add one for \0
    if(trimmed == NULL) {
      fprintf(stderr, "%s: could not allocate string\n", FUNC_NAME);
    }

    jj = 0;
    for(ii = start; ii < end; ii++) {
      trimmed[jj] = line[ii];   // copy only valid parts of string
      jj++;
    }
    trimmed[jj] = '\0';   // add null terminator

    // copy trimmed into line, so that trimmed can be free'd
    strncpy(line, trimmed, size);
    free(trimmed);
    return line;
  }


  /*
   * @brief Takes an argument list and checks if the first argument is a
   * built in command such as exit, cd, jobs, etc. If the command is a
   * built in command this function will handle the command and then return
   * true. If the first argument is NOT a built in command this function will
   * return false.
   *
   * @param sh The shell
   * @param argv The command to check
   * @return True if the command was a built in command
   */
  bool do_builtin(struct shell *sh, char **argv) {
    const char *FUNC_NAME = "do_builtin";
    const char *EXIT = "exit";
    const char *CD = "cd";
    const char *HISTORY = "history";

    bool isBuiltIn = false;

    if(strncmp(argv[0], EXIT, strlen(EXIT) + 1) == 0) {
      fprintf(stderr, "%s: exiting...\n", FUNC_NAME);
      isBuiltIn = true;
      sh_destroy(sh);
      exit(0);
    }
    if(strncmp(argv[0], CD, strlen(CD) + 1) == 0) {
      fprintf(stderr, "%s: changing dir...\n", FUNC_NAME);
      isBuiltIn = true;
      change_dir(argv);
    }
    if(strncmp(argv[0], HISTORY, strlen(HISTORY) + 1) == 0) {
      fprintf(stderr, "%s: showing hist...\n", FUNC_NAME);
      isBuiltIn = true;
      HIST_ENTRY **hist = history_list();
      if(hist == NULL) {
        fprintf(stderr, "%s: no history\n", FUNC_NAME);
      } else {
        for(int ii = 0; ii < history_length; ii++) {
          fprintf(stdout, "  %-3d   %s\n", ii, hist[ii]->line);
        }
      }
    }
    return isBuiltIn;
  }

  /*
   * @brief Initialize the shell for use. Allocate all data structures
   * Grab control of the terminal and put the shell in its own
   * process group. NOTE: This function will block until the shell is
   * in its own program group. Attaching a debugger will always cause
   * this function to fail because the debugger maintains control of
   * the subprocess it is debugging.
   *
   * @param sh
   */
  void sh_init(struct shell *sh) {
    const char *FUNC_NAME = "sh_init";
    // actually!! already initalized in main
    // sh = (struct shell*)malloc(sizeof(struct shell));
    // if(sh == NULL) {
    //   fprintf(stderr, "%s: could not allocate shell\n", METHOD_NAME);
    // }
    // fprintf(stderr, "%s: allocated shell\n", METHOD_NAME);

    sh->shell_is_interactive = 0;
    sh->shell_pgid = 0; // process group id
    
    // set up the termios struct??
    errno = 0;
    int rVal = tcgetattr(0, &(sh->shell_tmodes));
    if(rVal == -1) {
      perror("tcgetattr");
    }

/*  // generated from onyx defaults
    sh->shell_tmodes.c_iflag = 0x4d00;
    sh->shell_tmodes.c_oflag = OPOST | ONLCR;   // 0x5
    sh->shell_tmodes.c_cflag = 0xbf;
    sh->shell_tmodes.c_lflag = 0x8a3b;
    sh->shell_tmodes.c_line = 0x0;

    // there are only 17 defined control characters
    sh->shell_tmodes.c_cc[0] = VKILL;   // 0x3
    sh->shell_tmodes.c_cc[1] = 0x1c;
    sh->shell_tmodes.c_cc[2] = VSTART; // 0x8
    sh->shell_tmodes.c_cc[3] = 0x15;
    sh->shell_tmodes.c_cc[4] = VEOF;    // 0x4
    sh->shell_tmodes.c_cc[5] = VINTR;   // 0x0
    sh->shell_tmodes.c_cc[6] = VQUIT;  // 0x1
    sh->shell_tmodes.c_cc[7] = VINTR;   // 0x0
    sh->shell_tmodes.c_cc[8] = 0x11;
    sh->shell_tmodes.c_cc[9] = 0x13;
    sh->shell_tmodes.c_cc[10] = 0x1a;
    sh->shell_tmodes.c_cc[11] = 0x0;
    sh->shell_tmodes.c_cc[12] = 0x12;
    sh->shell_tmodes.c_cc[13] = VLNEXT; // 0xf
    sh->shell_tmodes.c_cc[14] = 0x17;
    sh->shell_tmodes.c_cc[15] = 0x16;
    sh->shell_tmodes.c_cc[16] = 0x0;

    sh->shell_tmodes.c_ispeed = 0xf;
    sh->shell_tmodes.c_ospeed = 0xf;
*/

    sh->shell_terminal = 0;
    char *prompt = get_prompt("MY_PROMPT");
    if(prompt == NULL) {
      fprintf(stderr, "%s: could not allocate string\n", FUNC_NAME);
      // FIXY exit?
    }
    sh->prompt = prompt;

  }

  /*
   * @brief Destroy shell. Free any allocated memory and resources and exit
   * normally.
   *
   * @param sh
   */
  void sh_destroy(struct shell *sh) {
    if(sh != NULL) {
      free(sh->prompt);
      // free(sh);
      // sh = NULL;
    }
  }

  /*
   * @brief Parse command line args from the user when the shell was launched
   *
   * @param argc Number of args
   * @param argv The arg array
   */
  void parse_args(int argc, char **argv) {
    // Use getopt() to process command line arguments
    int opt = 0;
    while((opt = getopt(argc, argv, "v")) != -1) {
        switch(opt) {
        case 'v':
            // print version
            fprintf(stdout, "version: %d.%d\n", lab_VERSION_MAJOR, lab_VERSION_MINOR);
            exit(0);
            break;
        default:  // invalid arg
            // error handling: getopt prints its own errors for invalid option, so cute!
        break;
        }
    }
  }