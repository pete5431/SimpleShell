#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "myshell.h"

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

enum Operator {

	REDIRECT_INPUT = 9,
	REDIRECT_OUTPUT_TRUNC = 10,
	REDIRECT_OUTPUT_APP = 11,
	PIPE = 12,
	BACKGROUND = 13
};


char** tokenize(char* input, char* delimiter);
void free_token_array(char** token_array);
void parse_input(char** token_array, char* user_input);
void set_shell_path();
char** add_argument(char**, char*, int);

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
	
	set_shell_path();

	while(1){

		if(bash_mode_on){

			getline(&user_input, &size, bash_fp);

			if(feof(bash_fp)){
				exit(0);
			}

			printf("read from file: %s\n", user_input);

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

	strncpy(user_copy, user_input, strlen(user_input) + 1);

	char** token_array = (char**)malloc(sizeof(char*));

	char* token = strtok(user_copy, delimiter);

	int i = 0;

	while(token != NULL){
	
		char* token_copy = malloc((strlen(token) + 1) * sizeof(char));	
		strncpy(token_copy, token, strlen(token) + 1);
		token_array[i] = token_copy;
		token = strtok(NULL, delimiter);
		i++;
		token_array = realloc(token_array, (i + 1) * sizeof(char*));
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

	char** new_arguments = realloc(arguments, (num_arguments + 1) * sizeof(char*));

        if(new_arguments == NULL){

		printf("Error: Memory allocation failed.\n");
		
		if(arguments != NULL){
			free(arguments);
		}

	}
	else arguments = new_arguments;

	arguments[num_arguments] = add_on;

	//printf("Added arg: %s at index %d, id: %p\n", arguments[num_arguments], num_arguments, arguments[num_arguments]);

	return arguments;
}

void parse_input(char** token_array, char* user_input){

	int i = 0;

	char* command = NULL;

	char** arguments = NULL;

	int look_for_arguments = 0;

	int num_arguments = 0;

	int operator_detected = 0;

	while(token_array[i] != NULL){

		if(!look_for_arguments){

			if(is_built_in(token_array[i]) != -1){

				command = malloc((strlen(token_array[i]) + 1) * sizeof(char));
                        	strncpy(command, token_array[i], strlen(token_array[i]) + 1);
                        	look_for_arguments = 1;

			}
			else{

				if(is_valid_external(token_array[i]) != -1){

                                	look_for_arguments = 1;

					command = malloc((strlen(token_array[i]) + 1) * sizeof(char));
					strncpy(command, token_array[i], strlen(token_array[i]) + 1);
				
					arguments = add_argument(arguments, token_array[i], num_arguments);

                                	num_arguments++;

				}
				else{

					command = malloc((strlen(token_array[i]) + 1) * sizeof(char));
                                        strncpy(command, token_array[i], strlen(token_array[i]) + 1);

                                        arguments = add_argument(arguments, token_array[i], num_arguments);

                                        num_arguments++;
				}

			}

		}
		else if(look_for_arguments){

			if(is_operator(token_array[i]) == -1){

				arguments = add_argument(arguments, token_array[i], num_arguments);
	
				num_arguments++;

			}
			else{

				if(is_operator(token_array[i]) == REDIRECT_OUTPUT_TRUNC && !operator_detected){

					operator_detected = 1;
					if(token_array[i + 1] != NULL){
						arguments = add_argument(arguments, token_array[i + 1], num_arguments);
						num_arguments++;
						i++;
					}

				} 	

			}
		}
		i++;
	}

	arguments = add_argument(arguments, NULL, num_arguments);

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
			else command_echo(arguments);

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

			if(num_arguments == 1){
				command_dir(arguments[0], -1);
			}
		}
		else if(is_built_in(command) == EXIT_COMMAND){

			if(num_arguments == 0){
				
				free(user_input);
				free_token_array(token_array);
				exit(0);

			}
		}
		else{

			command_external(command, arguments);

		}
	}

	free(command);
	free(arguments);
}
