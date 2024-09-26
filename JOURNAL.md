# CS452 - P1

## Requirements
* design ideas for each task
* function achieved with shell
* if AI tools for debugging, need to paste snapshots of prompts in summary

* empty commands should not cause segfault or memory leak, just display another prompt

## Questions and Issues
* what do we do with the shell struct, how should it be initialized?,
  * what does the shell mean when it says it should be in its own process group?
  * what do the fields of the shell struct mean?, when do we set them?

* is the format of my main function alright?
* what do the signals mean?
* what is a process tree?
* are we completing task 8 (signals), task 9 (background processes), task 10 (jobs command)
* format of the reflection?
  * I will be includign references

* if I do not free the shell, it complains to me about lost memory, if I do free the shell, it tells me I cannot free non malloc'd items
  * fixed, 2024-09-25. it's because it's being allocated in main, doesn't need to be reallocated, just the information needs to be added into the struct


## Notes

### 2024-09-26
* I cannot handle spaces or empty input
  * fixed via putting all execution within the "check if this line is not null"
* https://stackoverflow.com/questions/78125/why-cant-i-convert-char-to-a-const-char-const-in-c

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
