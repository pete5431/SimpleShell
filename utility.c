#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
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
	BOTH_IN_OUT_TRUNC = 12,
	BOTH_IN_OUT_APP = 13,
        PIPE = 14,
        BACKGROUND = 15
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

	int valid = -1;

	if(command != NULL){

		char bin[6] = "/bin/";

		char user_bin[10] = "/usr/bin/";

		char* bin_path = malloc(strlen(command) + 7 * sizeof(char));
		char* user_bin_path = malloc(strlen(command) + 11 * sizeof(char));

		strncpy(bin_path, bin, 5);
		strncpy(user_bin_path, user_bin, 10);

		strncat(bin_path, command, strlen(command) + 1);
		strncat(user_bin_path, command, strlen(command) + 1);

		if(access(bin_path, X_OK) != -1){

			command = (char*) realloc(command, strlen(bin_path) + 1 * sizeof(char));

			strncpy(command, bin_path, strlen(bin_path) + 1);

			valid = 1;

		}
		else if(access(user_bin_path, X_OK) != -1){

			command = (char*) realloc(command, strlen(user_bin_path) + 1 * sizeof(char));

			strncpy(command, user_bin_path, strlen(user_bin_path) + 1);

			valid = 1;
			
		}
		else if(access(command, X_OK) != -1){

			valid = 1;

		}
		free(bin_path);
		free(user_bin_path);
		return valid;
	}
	return valid;
}

void command_cd(char* directory_name){

	if(directory_name == NULL){

		printf("%s\n", getenv("PWD"));

	}	
	else{

		if(chdir(directory_name) == 0){
	
			char* cwd = getcwd(NULL, 0);

			setenv("PWD", cwd, 1);	

			free(cwd);
		}
		else{
			printf("Error: directory not found\n");
		}

	}
}

void command_clr(){

	printf("\033[H\033[J");

}

void command_external(char** argv, char* out_filename, char* in_filename, int state){

	int original_stdin = dup(0);
	int original_stdout = dup(1);	

	if(state != -1){

		if(state == REDIRECT_OUTPUT_TRUNC || state == BOTH_IN_OUT_TRUNC){
			int out_file = open(out_filename, O_WRONLY|O_CREAT|O_TRUNC, S_IRWXU|S_IRWXG|S_IRWXO);
			if(out_file == -1){
				printf("%s not found.\n", out_filename);
				return;
			}
			dup2(out_file, 1);
			close(out_file);
		}
		
		if(state == REDIRECT_OUTPUT_APP || state == BOTH_IN_OUT_APP){
			int out_file = open(out_filename, O_WRONLY|O_CREAT|O_APPEND, S_IRWXU|S_IRWXG|S_IRWXO);
			if(out_file == -1){
				printf("%s not found.\n", out_filename);
				return;
			}
                        dup2(out_file, 1);
                        close(out_file);
		}

		if(state == REDIRECT_INPUT || state == BOTH_IN_OUT_APP || state == BOTH_IN_OUT_TRUNC){
			int in_file = open(in_filename, O_RDONLY, S_IRWXU|S_IRWXG|S_IRWXO);
			if(in_file == -1){
				printf("%s not found.\n", in_filename);
				return;
			}
			dup2(in_file, 0);
			close(in_file);
		}
	}

	pid_t pid = fork();

	if(pid == -1){
		printf("Forking error.\n");
		return;
	}			
	else if(pid == 0){

		if(execvp(argv[0], argv) < 0){
			printf("%s not valid command.\n", argv[0]);
			exit(0);
		}	

	}
	else{

		if(state != BACKGROUND){
			int status = 0;
			waitpid(pid, &status, 0);
		}
	}
	dup2(original_stdin, 0);
	dup2(original_stdout, 1);
}

void command_external_pipe(char** arg1, char** arg2){

	int fd[2];

	pipe(fd);

	pid_t pid_1 = fork();

	if(pid_1 == -1){
		printf("Forking Error.\n");
		return;
	}
	else if(pid_1 == 0){
		close(fd[0]);
		dup2(fd[1], STDOUT_FILENO);
		close(fd[1]);
		if(execvp(arg1[0], arg1)){
			printf("Error running arg1.\n");
			exit(0);
		}	
	}
	else{

		pid_t pid_2 = fork();
	
		if(pid_2 == -1){
			printf("Forking Error.\n");
			return;
		}
		else if(pid_2 == 0){
			close(fd[1]);
			dup2(fd[0], STDIN_FILENO);
			close(fd[0]);
			if(execvp(arg2[0], arg2)){
				printf("Error running arg2.\n");
				exit(0);
			}
		}
		else{
			int status;
			close(fd[1]);
			close(fd[0]);
			waitpid(pid_1, &status, 0);
			waitpid(pid_2, &status, 0);
		}
	}
}

void command_help(char* command, char* out_filename, int state){

        FILE* out_fp = NULL;

        int original_stdout = dup(1);

        if(state != -1){

                if(state == REDIRECT_OUTPUT_TRUNC){
                        out_fp = fopen(out_filename, "w");
                }
                else if(state == REDIRECT_OUTPUT_APP){
                        out_fp = fopen(out_filename, "a+");
                }

                if(out_fp == NULL){
                        printf("Error: file not found.\n");
                        return;
                }
                dup2(fileno(out_fp), 1);
        }

	if(command == NULL){
		printf("Enter a command after help.\n");
	}
	else {

		int command_identity = is_built_in(command);

		if(command_identity == -1){
			printf("Command not built-in.\n");
			return;
		}

		if(command_identity == CD_COMMAND){
			printf("Command cd: \n");
			printf("cd accepts one or no arguments.\n");
			printf("If no arguments, prints current directory.\n");
			printf("If one arguments, changes to the given directory if valid.\n");
		}
		else if(command_identity == ENVIRON_COMMAND){
			printf("Command environ: \n");
			printf("environ prints out environment variables.\n");
		}
		else if(command_identity == ECHO_COMMAND){
			printf("Command echo: \n");
			printf("echo prints out to screen everything after it.\n");
		}
		else if(command_identity == DIR_COMMAND){
			printf("Command dir: \n");
			printf("dir prints out to screen the directory contents.\n");
			printf("If no arguments, it prints out the current directory contents");
			printf("If arguments present, and the directory is valid, it prints out the contents.\n");
		}
		else if(command_identity == HELP_COMMAND){
			printf("Command help: \n");
			printf("The command that prints out the manual for a built-in command.\n");
			printf("Needs one argument, which is the command to print manual for.\n");
		}
		else if(command_identity == PAUSE_COMMAND){
			printf("Command pause: \n");
			printf("The command pause pauses the shell until enter is entered.\n");
		}
		else if(command_identity == EXIT_COMMAND){
			printf("Command exit: \n");
			printf("The command exit exits the shell.\n");
		}
	}	
	if(out_fp != NULL){
                fclose(out_fp);
        }
        dup2(original_stdout, 1);
}

void command_echo(char** arguments, char* out_filename, int state){

	int i = 0;

        FILE* out_fp = NULL;

        int original_stdout = dup(1);	

	if(state != -1){

                if(state == REDIRECT_OUTPUT_TRUNC){
                        out_fp = fopen(out_filename, "w");
                }
                else if(state == REDIRECT_OUTPUT_APP){
                        out_fp = fopen(out_filename, "a+");
                }

                if(out_fp == NULL){
                        printf("Error: file not found.\n");
                        return;
                }
                dup2(fileno(out_fp), 1);
        }

	while(arguments[i] != NULL){

		printf("%s ", arguments[i]);
		i++;
	}

	printf("\n");

	if(out_fp != NULL){
		fclose(out_fp);	
	}
	dup2(original_stdout, 1);
}

void command_environ(char* out_filename, int state){

	FILE* out_fp = NULL;

        int original_stdout = dup(1);

        if(state != -1){

                if(state == REDIRECT_OUTPUT_TRUNC){
                        out_fp = fopen(out_filename, "w");
                }
                else if(state == REDIRECT_OUTPUT_APP){
                        out_fp = fopen(out_filename, "a+");
                }

                if(out_fp == NULL){
                        printf("Error: file not found.\n");
                        return;
                }
                dup2(fileno(out_fp), 1);
        }

	printf("USER=%s\n", getenv("USER"));
	printf("PATH=%s\n", getenv("PATH"));
	printf("PWD=%s\n", getenv("PWD"));
	printf("LANG=%s\n", getenv("LANG"));
	printf("HOME=%s\n", getenv("HOME"));

	if(out_fp != NULL){
		fclose(out_fp);
	}
	dup2(original_stdout, 1);
}

void command_pause(){

	fflush(stdin);
	while(getchar() != '\n'){
		
	}
}

void command_dir(char** arguments, char* out_filename, int state){

	DIR* dir;

	struct dirent *d;

	int i = 0;

	FILE* out_fp = NULL;

	int original_stdout = dup(1);

	if(state != -1){

		if(state == REDIRECT_OUTPUT_TRUNC){
			out_fp = fopen(out_filename, "w");
		}
		else if(state == REDIRECT_OUTPUT_APP){
			out_fp = fopen(out_filename, "a+");
		}

		if(out_fp == NULL){
			printf("Error: file not found.\n");
			return;
		}
		dup2(fileno(out_fp), 1);
	}

	while(arguments[i] != NULL){

		dir = opendir(arguments[i]);

		if(dir == NULL){

			printf("Error: %s not found.\n", arguments[i]);

			break;

		}

		printf("%s:\n", arguments[i]);

		while((d = readdir(dir)) != NULL){

			if(strcmp(d->d_name, ".") == 0 || strcmp(d->d_name, "..") == 0){

			}	
			else printf("%s\n", d->d_name);
		}
	
		printf("\n");
	
		i++;
	}
	if(out_fp != NULL){
		fclose(out_fp);
	}
	dup2(original_stdout, 1);
}
