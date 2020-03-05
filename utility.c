#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include "myshell.h"

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
