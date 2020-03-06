#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
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

int is_built_in(char* token){

        if(strcmp(token, "exit") == 0){

                return EXIT_COMMAND;

        }
        else if(strcmp(token, "cd") == 0){

                return CD_COMMAND;

        }
        else if(strcmp(token, "clr") == 0){

                return CLR_COMMAND;

        }
        else if(strcmp(token, "help") == 0){

                return HELP_COMMAND;

        }
        else if(strcmp(token, "environ") == 0){

                return ENVIRON_COMMAND;

        }
        else if(strcmp(token, "echo") == 0){

                return ECHO_COMMAND;

        }
        else if(strcmp(token, "dir") == 0){

                return DIR_COMMAND;

        }
        else if(strcmp(token, "pause") == 0){

                return PAUSE_COMMAND;

        }
        else return -1;
}

int is_operator(char* token){

        if(strcmp(token, ">") == 0){

                return REDIRECT_OUTPUT_TRUNC;

        }
        else if(strcmp(token, ">>") == 0){

                return REDIRECT_OUTPUT_APP;

        }
        else if(strcmp(token, "<") == 0){

                return REDIRECT_INPUT;

        }
        else if(strcmp(token, "&") == 0){

                return BACKGROUND;

        }
        else if(strcmp(token, "|") == 0){

                return PIPE;

        }
        else return -1;
}

int is_valid_external(char* command){

	printf("The id of command before: %p\n", command);

	if(command != NULL){

		char bin[6] = "/bin/";

		char user_bin[11] = "/usr/bin/";

		char* bin_path = malloc(strlen(command) + 7 * sizeof(char));
		char* user_bin_path = malloc(strlen(command) + 12 * sizeof(char));

		strncpy(bin_path, bin, 5);
		strncpy(user_bin_path, user_bin, 10);

		strncat(bin_path, command, strlen(command) + 1);
		strncat(user_bin_path, command, strlen(command) + 1);

		if(access(bin_path, X_OK) != -1){

			printf("%s\n", bin_path);

			command = (char*) realloc(command, strlen(bin_path) + 1 * sizeof(char));

			strncpy(command, bin_path, strlen(bin_path) + 1);

			printf("New command id: %p\n", command);

			free(bin_path);
			free(user_bin_path);

			return 1;

		}
		else if(access(user_bin_path, X_OK) != -1){

			printf("%s\n", user_bin_path);

			command = (char*) realloc(command, strlen(user_bin_path) + 1 * sizeof(char));

			strncpy(command, user_bin_path, strlen(user_bin_path) + 1);

			printf("New command id: %p\n", command);
			
			free(bin_path);
			free(user_bin_path);			

			return 1;
			
		}
		else{

			printf("No accessable\n");
			free(bin_path);
			free(user_bin_path);
			return -1;

		}

	}
	return -1;
}

void command_cd(char* directory_name){

	if(directory_name == NULL){

		printf("%s\n", getenv("PWD"));

	}	
	else{

		if(chdir(directory_name) == 0){
	
			char cwd[100];

			getcwd(cwd,100);

			setenv("PWD", cwd, 1);	
		}
		else{
			printf("Error: directory not found\n");
		}

	}
}

void command_clr(){

	printf("\033[H\033[J");

}

void command_external(char* command, char** argv){

	int pid = fork();

	if(pid == -1){
		printf("Forking error.\n");
	}			
	else if(pid == 0){

		if(execv(command, argv) < 0){
			exit(0);
		}	

	}
	else{

		int status = 0;
		wait(&status);
		
	}
}

void command_help(char* command){

	int print_all = 0;

	if(command == NULL){

		print_all = 1;

	}
	
	if(command != NULL){

		if(is_built_in(command) == CD_COMMAND){

			printf("Command cd: \n");
			printf("cd accepts one or no arguments.\n");
			printf("If no arguments, prints current directory.\n");
			printf("If one arguments, changes to the given directory if valid.\n");

		}
	}	
}

void command_echo(char** arguments){

	int i = 0;

	while(arguments[i] != NULL){

		printf("%s ", arguments[i]);
		i++;
	}

	printf("\n");
}
