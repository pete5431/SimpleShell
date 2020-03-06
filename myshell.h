#ifndef MY_SHELL_H
#define MY_SHELL_H

void command_cd(char*);
void command_clr();
void command_external(char*, char**);
void command_help(char*);
int is_built_in(char*);
int is_operator(char*);
int is_valid_external(char*);

#endif
