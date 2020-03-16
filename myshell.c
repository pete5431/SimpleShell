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

// Array for environment variables.
extern char* environ[];

// Tokenizes the input with the delimiter, and returns an array of string tokens.
char** tokenize(char* input, char* delimiter);
// Frees the array of string tokens.
void free_token_array(char** token_array);
// Parses the array of string tokens, and calls the appropriate function based off command, and handle errors.
void parse_input(char** token_array, char* user_input);
// Add argument to the arguments array or pipe arguments array.
char** add_argument(char**, char*, int);
// Accepts a built in command, and decide which command function to call, and handle errors.
void execute_built_in(char*, char**, char*, int, int);
// Checks if input contains a pipe or background.
int contains_non_built_in_operator(char**);

int main(int argc, char* argv[]){

	// Indicates if bash mode is on, default is off.
	int bash_mode_on = 0;
	// File pointer for bash file.
	FILE* bash_fp;

	// If shell executable was executed with a batch file.
	if(argc == 2){
		
		// Open the batch file in read mode.
		bash_fp = fopen(argv[1], "r");

		// Check for if open failed. If so exit shell.
		if(bash_fp == NULL){

			printf("File open failed.\n");
			exit(0);
		}

		// Turn bash mode on if open file was successful.
		bash_mode_on = 1;

	}
	else if(argc > 2){
		// If arguments to shell executable was more than one.
		printf("Too many arguments to myshell.\n");
		exit(0);
	}

	// String for storing user input entered after shell prompt.
	char* user_input = NULL;
	// Size for getline.
	size_t size = 0;

	// The shell will continue running until the exit command is typed.
	while(1){

		// If bash mode is on read input from file instead.
		if(bash_mode_on){

			getline(&user_input, &size, bash_fp);

			// Upon reaching EOF, close file, and exit shell.
			if(feof(bash_fp)){
				fclose(bash_fp);
				exit(0);
			}

		}else{
			// Get input from stdin.

			printf("myshell%s> ", getenv("PWD"));

			getline(&user_input, &size, stdin);

		}

		// Tokenize the input by spaces into a array of tokens.
		char** token_array = tokenize(user_input, " \t\n");

		// Parse the token array to execute commands.
		parse_input(token_array, user_input);

		// Free the token array.
		free_token_array(token_array);
	}
	return 0;
}

// Tokenizes the user input into tokens by the given delimiter.
char** tokenize(char* user_input, char* delimiter){

	// Make a string to store a copy of user input in.
	char* user_copy = calloc(strlen(user_input) + 1, sizeof(char));

	// Check if user_copy was calloced correctly.
	if(user_copy == NULL){
		printf("Memory Allocation Failed.\n");
		return NULL;
	}

	// Copy user_input into user_copy.
	strncpy(user_copy, user_input, strlen(user_input) + 1);

	// Allocate memory for token array using malloc.
	char** token_array = (char**)malloc(sizeof(char*));

	// Check if allocation successful.
	if(token_array == NULL){
		printf("Memory Allocation Failed.\n");
		return NULL;
	}

	// Get the first token using strtok on the copy of user input.
	char* token = strtok(user_copy, delimiter);

	// Index of token array.
	int i = 0;

	// Keep tokenizing until the returned token is NULL.
	while(token != NULL){
	
		// Allocate memory for a copy of the token.
		char* token_copy = malloc((strlen(token) + 1) * sizeof(char));	
		// Check if successful.
		if(token_copy == NULL){
			printf("Memory Allocation Failed.\n");
			return NULL;
		}
		// Copy the contents of token into token_copy.
		strncpy(token_copy, token, strlen(token) + 1);
		// Set the current index in token array equal to the token copy.
		token_array[i] = token_copy;
		// Get the next token.
		token = strtok(NULL, delimiter);
		// Increment the index.
		i++;

		// Reallocate more memory in token array for one more token, including space at the end for NULL.
		char** new_token_array = realloc(token_array, (i + 1) * sizeof(char*));
		
		// Check if successful, and if it is, set old token array to new one.
		if(new_token_array == NULL){
			printf("Memory Allocation Failed.\n");
			return NULL;
		}else token_array = new_token_array;

	}

	// Set the last index to NULL.
	token_array[i] = NULL;

	// Free the user copy of user input.
	free(user_copy);

	// Return the token array.
	return token_array;
}

// Frees the passed token array.
void free_token_array(char** token_array){

	int i = 0;

	// Frees each token, and then the array itself.
	while(token_array[i] != NULL){
		free(token_array[i]);
		i++;
	}
	free(token_array);
}

// Add arguments to the passed arguments array, with the specified string to add on, and the current number of arguments.
char** add_argument(char** arguments, char* add_on, int num_arguments){

	// Reallocs more memory for added arguments. The + 2 is so that theres space for NULL.
	char** new_arguments = realloc(arguments, (num_arguments + 2) * sizeof(char*));

	// Check to make sure reallocation was successful. Set old arguments to new arguments if successful.
        if(new_arguments == NULL){

		printf("Error: Memory allocation failed.\n");
		
		if(arguments != NULL){
			free(arguments);
		}
	}
	else arguments = new_arguments;

	// Add on the new argument.
	arguments[num_arguments] = add_on;

	// Add NULL to end of the arguments.
	arguments[num_arguments + 1] = NULL;

	// Return the arguments array.
	return arguments;
}

// Checks if the token array contains pipe or run in background.
int contains_non_built_in_operator(char** token_array){

	int i = 0;

	while(token_array[i] != NULL){
		// Check if any token equals | , & or <.
		if(strcmp(token_array[i], "|") == 0 || strcmp(token_array[i], "&") == 0 || strcmp(token_array[i], "<") == 0){
			return 1;
		}
		i++;
	}
	return 0;
}

// Takes the token array and parses it.
void parse_input(char** token_array, char* user_input){

	// Index for the token array.
	int i = 0;

	// Contains the built in command if built in command is detected.
	char* built_in_command = NULL;

	// Contains the arguments plus external command at index 0.
	char** arguments = NULL;
	
	// Contains the command and arguments on other end of pipe.
	char** pipe_arg = NULL;

	// The output filename after a output redirect.
	char* out_filename = NULL;

	// The input filename after a input redirect.
	char* in_filename = NULL;

 	// num_arguments is the number of arguments currently in either arguments or pipe_arg.
	int num_arguments = 0;

	// The state of the command, either redirection, background or pipe. -1 by default to indicate normal state.
	int state = -1;

	// pipe_on indicates if pipe is detected, which prompts the while look to instead add arguments to pipe_arg.
	// use_built_in is to indicate if pipe or background is present, use non built-in version of built-in command.
	int pipe_on = 0, use_built_in = 1;

	int count_redirect = 0;

	// Checks if token array contains pipe or background, and sets use_built_in to false if detected.
	if(contains_non_built_in_operator(token_array)){
		use_built_in = 0;
	}

	// Loop through all tokens of token array until reaching NULL.
	while(token_array[i] != NULL){

		if(i == 0 && is_operator(token_array[i]) != -1){
			printf("Improper use of %s\n", token_array[i]);
			return;
		}
		else if(use_built_in && is_built_in(token_array[i]) != -1){

			built_in_command = token_array[i];
			use_built_in = 0;
		}
		else{

			int operator_identity = is_operator(token_array[i]);

			if(operator_identity == -1){

				if(pipe_on){
					pipe_arg = add_argument(pipe_arg, token_array[i], num_arguments);
					num_arguments++;
				}
				else{

					arguments = add_argument(arguments, token_array[i], num_arguments);
					num_arguments++;
				}
				use_built_in = 0;
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
					
					state = BACKGROUND;

					command_external(arguments, in_filename, out_filename, state);
					num_arguments = 0;
					state = -1;
					free(arguments);
					arguments = NULL;

				}
				else if(operator_identity == PIPE){
				
					if(token_array[i + 1] == NULL){
						printf("No argument after pipe.\n");
						return;
					}
	
					pipe_on = 1;		
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
				command_help(arguments[0], out_filename, state);
			}
			else if(num_arguments == 0){
				printf("No arguments after help.\n");
			}
			else if(num_arguments > 1){
				printf("Too many arguments to help.\n");
			}
                }
		else if(is_built_in(command) == ECHO_COMMAND){

                       	command_echo(arguments, out_filename, state);

                }
                else if(is_built_in(command) == ENVIRON_COMMAND){

                        command_environ(out_filename, state);

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
