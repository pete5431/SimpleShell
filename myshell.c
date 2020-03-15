#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "myshell.h"

// enums for the built-in commands.
enum Command {

	CD_COMMAND = 1,
	DIR_COMMAND = 2,
	CLR_COMMAND = 3,	
	ECHO_COMMAND = 4,
	ENVIRON_COMMAND = 5,
	PAUSE_COMMAND = 6,
	HELP_COMMAND = 7,
	EXIT_COMMAND = 8
};

// enums for the operators.
enum Operator {

	REDIRECT_INPUT = 9,
	REDIRECT_OUTPUT_TRUNC = 10,
	REDIRECT_OUTPUT_APP = 11,
	BOTH_IN_OUT_TRUNC = 12,
	BOTH_IN_OUT_APP = 13,
	PIPE = 14,
	BACKGROUND = 15
};

char** tokenize(char* input, char* delimiter);
void free_token_array(char** token_array);
void parse_input(char** token_array, char* user_input);
char** add_argument(char**, char*, int);
void execute_built_in(char*, char**, char*, int, int);

void set_shell_path();

int main(int argc, char* argv[]){

	int bash_mode_on = 0;
	FILE* bash_fp;

	if(argc == 2){
		
		bash_fp = fopen(argv[1], "r");

		if(bash_fp == NULL){

			printf("File open failed.\n");
			exit(0);

		}

		bash_mode_on = 1;

	}
	else if(argc > 2){
		
		printf("Too many arguments to myshell.\n");
		exit(0);
	}

	char* user_input = NULL;
	size_t size = 0;

	while(1){

		if(bash_mode_on){

			getline(&user_input, &size, bash_fp);

			if(feof(bash_fp)){
				fclose(bash_fp);
				exit(0);
			}

		}else{

			printf("myshell%s> ", getenv("PWD"));

			getline(&user_input, &size, stdin);

		}

		char** token_array = tokenize(user_input, " \t\n");

		parse_input(token_array, user_input);

		free_token_array(token_array);
	}
	return 0;
}

char** tokenize(char* user_input, char* delimiter){

	char* user_copy = calloc(strlen(user_input) + 1, sizeof(char));

	if(user_copy == NULL){
		printf("Memory Allocation Failed.\n");
		return NULL;
	}

	strncpy(user_copy, user_input, strlen(user_input) + 1);

	char** token_array = (char**)malloc(sizeof(char*));

	if(token_array == NULL){
		printf("Memory Allocation Failed.\n");
		return NULL;
	}

	char* token = strtok(user_copy, delimiter);

	int i = 0;

	while(token != NULL){
	
		char* token_copy = malloc((strlen(token) + 1) * sizeof(char));	
		if(token_copy == NULL){
			printf("Memory Allocation Failed.\n");
			return NULL;
		}
		strncpy(token_copy, token, strlen(token) + 1);
		token_array[i] = token_copy;
		token = strtok(NULL, delimiter);
		i++;

		char** new_token_array = realloc(token_array, (i + 1) * sizeof(char*));
		
		if(new_token_array == NULL){
			printf("Memory Allocation Failed.\n");
			return NULL;
		}else token_array = new_token_array;

	}

	token_array[i] = NULL;

	free(user_copy);

	return token_array;
}

void free_token_array(char** token_array){

	int i = 0;

	while(token_array[i] != NULL){
		free(token_array[i]);
		i++;
	}
	free(token_array);
}

void set_shell_path(){

	char cwd[100];

	getcwd(cwd, 100);

	char shell_name[9] = "/myshell";

	strncat(cwd, shell_name, 9);

	setenv("shell", cwd, 1); 
}

char** add_argument(char** arguments, char* add_on, int num_arguments){

	char** new_arguments = realloc(arguments, (num_arguments + 2) * sizeof(char*));

        if(new_arguments == NULL){

		printf("Error: Memory allocation failed.\n");
		
		if(arguments != NULL){
			free(arguments);
		}

	}
	else arguments = new_arguments;

	arguments[num_arguments] = add_on;

	arguments[num_arguments + 1] = NULL;

	return arguments;
}

void parse_input(char** token_array, char* user_input){

	int i = 0;

	char* built_in_command = NULL;

	char** arguments = NULL;

	char** pipe_arg = NULL;

	char* out_filename = NULL;

	char* in_filename = NULL;

	int look_for_arguments = 0, num_arguments = 0;

	int state = -1;

	int pipe_on = 0;

	while(token_array[i] != NULL){

		if(!look_for_arguments){

			if(is_built_in(token_array[i]) != -1){

				built_in_command = token_array[i];
                        	look_for_arguments = 1;

			}
			else{

				if(is_valid_external(token_array[i]) != -1){

                                	look_for_arguments = 1;
				
					if(pipe_on){

						pipe_arg = add_argument(pipe_arg, token_array[i], num_arguments);
						num_arguments++;

					}else {
			
						arguments = add_argument(arguments, token_array[i], num_arguments);

                                		num_arguments++;
					}

				}
				else{

					printf("%s not valid command.\n", token_array[i]);
					return;

				}

			}

		}
		else if(look_for_arguments){

			int operator_identity = -1;
		
			operator_identity = is_operator(token_array[i]);

			if(operator_identity == -1){

				if(pipe_on){
					pipe_arg = add_argument(arguments, token_array[i], num_arguments);
					num_arguments++;
				}
				else{

					arguments = add_argument(arguments, token_array[i], num_arguments);
	
					num_arguments++;
				}
			}
			else{
				if(operator_identity == REDIRECT_OUTPUT_TRUNC || operator_identity == REDIRECT_OUTPUT_APP || operator_identity == REDIRECT_INPUT){
					if(token_array[i + 1] != NULL){

						if(operator_identity == REDIRECT_OUTPUT_TRUNC){

							if(state == REDIRECT_INPUT){
								state = BOTH_IN_OUT_TRUNC;
							}
							else state = REDIRECT_OUTPUT_TRUNC;

							out_filename = token_array[i + 1];

						}	
						else if(operator_identity == REDIRECT_OUTPUT_APP){

							if(state == REDIRECT_INPUT){
								state = BOTH_IN_OUT_APP;
							}	
							else state = REDIRECT_OUTPUT_APP;

							out_filename = token_array[i + 1];
						}
						else if(operator_identity == REDIRECT_INPUT){

							if(state == REDIRECT_OUTPUT_TRUNC){
								state = BOTH_IN_OUT_TRUNC;
							}
							else if(state == REDIRECT_OUTPUT_APP){
								state = BOTH_IN_OUT_APP;
							}
							else state = REDIRECT_INPUT;

							in_filename = token_array[i + 1];

						}
						i++;
					}
					else{	
						printf("No arguments after operator.\n");
						return;
					}
				}
				else if(operator_identity == BACKGROUND){

					if(arguments == NULL){
						printf("& used without argument.\n");
						return;
					}
					
					state = BACKGROUND;

					command_external(arguments, in_filename, out_filename, state);
					num_arguments = 0;
					look_for_arguments = 0;
					state = -1;
					free(arguments);
					arguments = NULL;

				}
				else if(operator_identity == PIPE){
					
					pipe_on = 1;		
					look_for_arguments = 0;
					num_arguments = 0;

				}
			}	
		}
		i++;
	}

	if(built_in_command == NULL && arguments == NULL){
		return;
	}
	else if(built_in_command != NULL){

		if(is_built_in(built_in_command) == EXIT_COMMAND && num_arguments == 0){
			free(user_input);
			free(token_array);
			exit(0);
		}
		else execute_built_in(built_in_command, arguments, out_filename, num_arguments, state);

	}
	else{
		if(pipe_on){
			command_external_pipe(arguments, pipe_arg);
		}else command_external(arguments, out_filename, in_filename, state);
	}
	free(arguments);
	free(pipe_arg);
}

void execute_built_in(char* command, char** arguments, char* out_filename, int num_arguments, int state){

	if(command != NULL){

                if(is_built_in(command) == CD_COMMAND){

                        if(num_arguments > 1){

                                printf("Too many arguments to cd.\n");

                        }
                        else if(num_arguments == 1){

                                command_cd(arguments[0]);

                        }
                        else if(num_arguments == 0){

                                command_cd(NULL);

                        }

                }
                else if(is_built_in(command) == CLR_COMMAND){

                        if(num_arguments == 0){

                                command_clr();

                        }

                }
                else if(is_built_in(command) == HELP_COMMAND){

                        if(num_arguments == 1){

                                command_help(arguments[0]);

                        }
                        else if(num_arguments == 0){

                                command_help(NULL);

                        }

                }
		else if(is_built_in(command) == ECHO_COMMAND){

                        if(num_arguments == 0){

                                printf("\n");
                        }
                        else command_echo(arguments, out_filename, state);

                }
                else if(is_built_in(command) == ENVIRON_COMMAND){

                        if(num_arguments == 0){
                                command_environ();
                        }

                }
                else if(is_built_in(command) == PAUSE_COMMAND){

                        if(num_arguments == 0){
                                command_pause();
                        }
                }
                else if(is_built_in(command) == DIR_COMMAND){
			
			if(num_arguments == 0){
				char* cwd = getcwd(NULL, 0);

				arguments = add_argument(arguments, cwd, num_arguments);

				command_dir(arguments, out_filename, state);

				free(cwd);
			}
                        else{
				command_dir(arguments, out_filename, state);
			}
                }
        }
}
