#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
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

