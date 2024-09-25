# CS452 - P1

## Requirements
* design ideas for each task
* function achieved with shell
* if AI tools for debugging, need to paste snapshots of prompts in summary

## Questions
* what do we do with the shell struct, how should it be initialized?
* if I do not free the shell, it complains to me about lost memory, if I do free the shell, it tells me I cannot free non malloc'd items

## Issues

## Notes
* sleepy
* https://stackoverflow.com/questions/298510/how-to-get-the-current-directory-in-a-c-program

### 2024-08-23
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
