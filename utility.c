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

// Enums for built in commands.
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

// Enums for operators.
enum Operator {

        REDIRECT_INPUT = 9,
        REDIRECT_OUTPUT_TRUNC = 10,
        REDIRECT_OUTPUT_APP = 11,
	BOTH_IN_OUT_TRUNC = 12,
	BOTH_IN_OUT_APP = 13,
        PIPE = 14,
        BACKGROUND = 15
};

// Array that contains all the environment variables.
extern char** environ;

// Check if passed token is a built in command. Return the appropriate enum, or -1 if not built in.
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

// Check if the token is a operator we care about, returns the appropriate enum, returns -1 if not an operator we care about.
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

// The function for the built in cd command. Accepts one parameter, the directory name.
void command_cd(char* directory_name){

	// If the directory name is NULL.
	if(directory_name == NULL){
		// If no directory name was given, simply print the current directory.
		printf("%s\n", getenv("PWD"));
	}	
	else{
		// Change the current directory using chdir, and check is successful.
		if(chdir(directory_name) == 0){
			// Get the current directory.
			char* cwd = getcwd(NULL, 0);
			// Make sure getcwd returned something.
			if(cwd == NULL){
				printf("Error: couldn't get cwd.\n");
				return;
			}
			// Set the PWD environment variable to new cwd. Check if failed.
			if(setenv("PWD", cwd, 1) < 0){
				printf("Error: couldn't update PWD.");
				free(cwd);
				return;
			}
			// Free the string holding cwd.
			free(cwd);
		}
		else{
			// Else directory cannot be found, and print error.
			printf("Error: directory not found\n");
			return;
		}

	}
}

// The function for command clr. Simply prints the following to clear the screen.
void command_clr(){

	printf("\033[H\033[J");

}

// The function for the external commands. Accepts a file name for either output or input redirection, and a state indicating which or if its background.
void command_external(char** argv, char* out_filename, char* in_filename, int state){

	// Save the original file descriptors for stdout and stdin.
	int original_stdin = dup(0);
	int original_stdout = dup(1);	

	// If the state is not normal.
	if(state != -1){
		// If the state is "<" or "< >" or "< >>".
		if(state == REDIRECT_INPUT || state == BOTH_IN_OUT_APP || state == BOTH_IN_OUT_TRUNC){
			// Obtain file descriptor for the input file using read only and some permission flags.
                        int in_file = open(in_filename, O_RDONLY, S_IRWXU|S_IRWXG|S_IRWXO);
			// Check if the open was successful.
                        if(in_file == -1){
                                printf("%s not found.\n", in_filename);
                                return;
                        }
			// Copy the in_file descriptor into stdin.
                        dup2(in_file, 0);
			// Close the in_file descriptor, not needed anymore.
                        close(in_file);
                }

		// If the state is ">" or "< >"
		if(state == REDIRECT_OUTPUT_TRUNC || state == BOTH_IN_OUT_TRUNC){
			// Obtain file descriptor for the output file. If it doesn't exist create it. Use trunc flag.
			int out_file = open(out_filename, O_WRONLY|O_CREAT|O_TRUNC, S_IRWXU|S_IRWXG|S_IRWXO);
			// Check if the open was successful.
			if(out_file == -1){
				// If not successful, set stdin back to normal in case input was successful.
				dup2(original_stdin, 0);
				printf("%s not found.\n", out_filename);
				return;
			}
			// Copy the out_file descriptor into stdout.
			dup2(out_file, 1);
			// Close the out_file descriptor, not needed anymore.
			close(out_file);
		}
		
		// If the state is ">>" or "< >>"
		if(state == REDIRECT_OUTPUT_APP || state == BOTH_IN_OUT_APP){
			// Obtain the file descriptor for the output file. Has to exist. Use appen flag.
			int out_file = open(out_filename, O_WRONLY|O_APPEND, S_IRWXU|S_IRWXG|S_IRWXO);
			// Check if the open was successful.
			if(out_file == -1){
				dup2(original_stdin, 0);
				printf("%s not found.\n", out_filename);
				return;
			}
			// Copy the out_file descriptor into stdout.
                        dup2(out_file, 1);
			// Close the out_file descriptor, not needed anymore.
                        close(out_file);
		}

	}

	// Fork a new child, and get the pid.
	pid_t pid = fork();

	// If pid is negative, print error and return. Change descriptors back to normal.
	if(pid < 0){
		printf("Forking error.\n");
		dup2(original_stdin, 0);
		dup2(original_stdout, 1);
		return;
	}			
	else if(pid == 0){
		// Child process.

		// Execute the command and arguments. If fail, change descriptors back to print message and exit.
		if(execvp(argv[0], argv) < 0){
			dup2(original_stdin, 0);
			dup2(original_stdout, 1);
			printf("%s not valid command.\n", argv[0]);
			exit(0);
		}	

	}
	else{
		// Parent process.

		// If the state is not background, wait for the child to terminate and reap it.
		if(state != BACKGROUND){
			int status = 0;
			waitpid(pid, &status, 0);
		}
	}
	// At the end change the descriptors back to normal.
	dup2(original_stdin, 0);
	dup2(original_stdout, 1);
}

// Function for external command piping, but only for single pipes. Therefore accepts the two sides of the pipe.
void command_external_pipe(char** arg1, char** arg2){

	// The fd array to pipe.
	int fd[2];

	// Pipe the fd array. Check for piping error.
	if(pipe(fd) == -1){
		printf("Piping error.\n");
		return;
	}

	// Fork the first child and get pid.
	pid_t pid_1 = fork();

	// Check for forking error.
	if(pid_1 < 0){
		printf("Forking Error.\n");
		close(fd[1]);
		close(fd[0]);
		return;
	}
	else if(pid_1 == 0){
		// Child 1 process.

		// Close the read end of pipe, because not needed.
		close(fd[0]);
		// Copy the write end of pipe into stdout.
		dup2(fd[1], STDOUT_FILENO);
		// Close the write end of the pipe.
		close(fd[1]);
		// Execute the first command and arguments.
		if(execvp(arg1[0], arg1)){
			printf("Error running arg1.\n");
			exit(0);
		}	
	}
	else{
		// Parent process.

		// Fork the second child and get pid.
		pid_t pid_2 = fork();
	
		// Check for forking error.
		if(pid_2 < 0){
			printf("Forking Error.\n");
			close(fd[1]);
			close(fd[0]);
			return;
		}
		else if(pid_2 == 0){
			// Child 2 process.
	
			// Close the write end of the pipe.
			close(fd[1]);
			// Copy the read end of the pipe into stdin.
			dup2(fd[0], STDIN_FILENO);
			// Close the read end of the pipe.
			close(fd[0]);
			// Execute the second command and arguments.
			if(execvp(arg2[0], arg2)){
				printf("Error running arg2.\n");
				exit(0);
			}
		}
		else{
			// Wait for both children, and close the pipe.
			int status;
			close(fd[1]);
			close(fd[0]);
			waitpid(pid_1, &status, 0);
			waitpid(pid_2, &status, 0);
		}
	}
}

// Function for the command help. Accepts a output filename for output redirect, and a state for what kind.
void command_help(char* command, char* out_filename, int state){

	// Default file pointer for the output file if needed.
        FILE* out_fp = NULL;

	// Save the orignal stdout fd.
        int original_stdout = dup(1);

	// If state is not normal.
        if(state != -1){

		// If state is ">".
                if(state == REDIRECT_OUTPUT_TRUNC){
			// Open the file in write mode.
                        out_fp = fopen(out_filename, "w");
                }
		// If the state is ">>"
                else if(state == REDIRECT_OUTPUT_APP){
			// Open the file in append mode.
                        out_fp = fopen(out_filename, "a+");
                }

		// If the fp is still NULL, print file not found and return.
                if(out_fp == NULL){
                        printf("Error: file not found.\n");
                        return;
                }
		// Copy the open file descriptor into stdout.
                dup2(fileno(out_fp), 1);
        }

	// If the command help was entered with no arguments.
	if(command == NULL){
		// State command help needs a command argument.
		printf("Enter a command after help.\n");
	}
	else {

		// Identify the built in command.
		int command_identity = is_built_in(command);

		// If not a built in, state it, and return.
		if(command_identity == -1){
			printf("Command not built-in.\n");
			if(out_fp != NULL){
				fclose(out_fp);
			}
			dup2(original_stdout, 1);
			return;
		}

		// Prints the help manual for the following commands.
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
			printf("The pause command cannot be entered with other arguments.\n");
		}
		else if(command_identity == EXIT_COMMAND){
			printf("Command exit: \n");
			printf("The command exit exits the shell.\n");
			printf("The exit command cannot be entered with other arguments.\n");
		}
		else if(command_identity == CLR_COMMAND){
			printf("Command clr: \n");
			printf("The command clr clears the screen.\n");
		}
	}	
	if(out_fp != NULL){
                fclose(out_fp);
        }
        dup2(original_stdout, 1);
}

void command_echo(char** arguments, char* out_filename, int state){

	// The following out redirect code is the same for the others as explained in help.

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

	// If no arguments, just print a newline. If redirect was on, change descriptor back to normal afterwards.
	if(arguments == NULL){
		printf("\n");
		dup2(original_stdout, 1);
		return;
	}

	// Loop through arguments and print them.
	while(arguments[i] != NULL){

		printf("%s ", arguments[i]);
		i++;
	}

	// Print new line afterwards.
	printf("\n");

	if(out_fp != NULL){
		fclose(out_fp);	
	}
	dup2(original_stdout, 1);
}

// Function for the environ command. 
void command_environ(char* out_filename, int state){

	// The following out redirect code is the same for the others as explained in help.

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

	// Starts at one because address needs to incremented by one after getting the first env.
	int i = 1;

	// Get the first env variable from environ.
	char *env_var = *environ;

	// Loop through environ, and print out each env variable.
	for( ; env_var; i++){
		printf("%s\n", env_var);
		env_var = *(environ + i);
	}

	if(out_fp != NULL){
		fclose(out_fp);
	}
	dup2(original_stdout, 1);
}

// Function for command pause.
void command_pause(){

	// Flush stdin.
	fflush(stdin);
	// Enter any key to unpause.
	while(getchar() != '\n'){
		
	}
}

// Function for the command dir.
void command_dir(char** arguments, char* out_filename, int state){

	// Directory pointer for opening file directories.
	DIR* dir;

	// Struct for reading through contents of the directory.
	struct dirent *d;

	int i = 0;

	// The following out redirect code is the same for the others as explained in help.

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

	// Loop through each directory name passed to dir.
	while(arguments[i] != NULL){

		// Open the directory.
		dir = opendir(arguments[i]);

		// If unsuccessful open, print error.
		if(dir == NULL){

			printf("Error: %s not found.\n", arguments[i]);

			if(out_fp != NULL){
				fclose(out_fp);
			}
			dup2(original_stdout, 1);

			return;

		}

		// Print the directory for which the contents are printed for.
		printf("%s:\n", arguments[i]);

		// While there are stuff to read from the directory.
		while((d = readdir(dir)) != NULL){

			// Avoid printing the . and .. in directories.
			if(strcmp(d->d_name, ".") == 0 || strcmp(d->d_name, "..") == 0){

			}	
			// Print the content name.
			else printf("%s ", d->d_name);
		}
		// Print new line to separate.
		printf("\n");
	
		i++;
	}
	if(out_fp != NULL){
		fclose(out_fp);
	}
	dup2(original_stdout, 1);
}
