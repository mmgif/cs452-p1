# CS452 - P1

## Requirements
* design ideas for each task
* function achieved with shell
* if AI tools for debugging, need to paste snapshots of prompts in summary

- [X] empty commands should not cause segfault or memory leak, just display another prompt

Task 7
* Think about using fork() to create a child process, with the parent waiting for the child to finish in the main.c. The child should replace its image with the command using execvp().

Task 8
* You want the shell to ignore signals like SIGINT and SIGTSTP, but child processes should still handle them normally. Use signal() to ignore the signals in the parent (maybe sh_init()?), and reset the default behavior in the child after fork().

Task 9
* Detects the presence of & to identify background jobs, then run these commands without waiting for them to complete. But we need to store the background process IDs for later tracking, allowing the shell to continue accepting new commands while the background job runs.

Task 10
* Store info about background jobs (PID and command) in a list or array. Use waitpid() to check if background processes have finished without blocking the shell. The jobs command should display all active background processes and their statuses.

## Questions and Issues
- [X] what do we do with the shell struct, how should it be initialized?,
  -> fine to init in main and pass as ptr

  * what does the shell mean when it says it should be in its own process group?
    -> use tcgetgrp() and tcsetgrp() to grab control of terminal
    -> 1) use getpid() to get shell process id, 2) use setpgid() to put shell in its own group, 3) use tcsetgroup() and tcgetgrp() to assign terminal to shell

  * what do the fields of the shell struct mean?, when do we set them?
    -> shell_is_interactive: check if shell is running interactively
    -> shell_pgid: use setpgid() to set process group
    -> shell_tmodes and shell_terminal: handles terminal modes and control

* what do the signals mean?
* what is a process tree?

* format of the reflection?
  * I will be includign references

* if I do not free the shell, it complains to me about lost memory, if I do free the shell, it tells me I cannot free non malloc'd items
  * fixed, 2024-09-25. it's because it's being allocated in main, doesn't need to be reallocated, just the information needs to be added into the struct

## Notes

### 2024-09-29
* [X] being done and beign seen done might be two different concepts 
* https://stackoverflow.com/questions/8090888/what-is-the-max-possible-length-of-a-pid-of-a-process-64-bit

### 2024-09-27
* fix multiple shell exits needed if cmd fails by exiting the child process on execvp failure
* adding in the signals stuff
* need to add in the pushing hte shell to its own process group, or else it DIE
* jumping to exit after a command occurs (fail or not), something to do with line reading?
  * probably a question of waht is in the buffer
  * 2024-09-28: nah, it had to do with relinquishign control to the child process and never askign for it back :thumbsup:

### 2024-09-26
* I cannot handle spaces or empty input
  * fixed via putting all execution within the "check if this line is not null"
* https://stackoverflow.com/questions/78125/why-cant-i-convert-char-to-a-const-char-const-in-c
* ssh into onyx, if on lab computers, need to ssh into onyx LOL
* https://www.geeksforgeeks.org/fork-execute-processes-bottom-using-wait/
* need to ignore or handle commands written while process is executing?

### 2024-09-25
* sleepy
* https://stackoverflow.com/questions/298510/how-to-get-the-current-directory-in-a-c-program
* https://unix.stackexchange.com/questions/404054/how-is-a-process-group-id-set

### 2024-09-23
* https://stackoverflow.com/questions/5935933/dynamically-create-an-array-of-strings-with-malloc
* does chdir do relative paths?
* getting better about adding print statements
* checking mallocs/frees

### 2024-09-22
* discovered shell init is not setting any of it variables? idk why
* moving on to trim white space

* yay it works
* https://www.digitalocean.com/community/tutorials/execvp-function-c-plus-plus
* https://stackoverflow.com/questions/16782715/termios-default-for-standard-terminal-session
* https://cplusplus.com/reference/cstring/strtok/

### 2024-09-21
* added custom prompt
* user input, rough draft (it kinda works?)
* missing null terminator in custom prompt strcpy :(

### 2024-09-17
* added print version, realized it needs to be in lab.c

### 2024-09-12
* added starter code
