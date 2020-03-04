#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "myshell.h"

char** tokenize(char* input, char* delimiter);
void free_token_array(char** token_array);
void parse_input(char** token_array, char* user_input);

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
	
	putenv("shell=OS-Shell/myshell");

	while(1){

		if(bash_mode_on){

			if(feof(bash_fp)){

				printf("End of file!\n");
				exit(0);
			}
	
			getline(&user_input, &size, bash_fp);

		}else{

			char* prompt = getenv("shell");
			printf("%s> ", prompt);

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

void parse_input(char** token_array, char* user_input){

	int i = 0;

	int NOT_FOUND = 0;

	while(token_array[i] != NULL){

		if(strcmp(token_array[i], "exit") == 0 && token_array[i + 1] == NULL){

			free(user_input);
			free(token_array);
			exit(0);

		}
		else if(strcmp(token_array[i], "cd") == 0){
		
			printf("cd entered\n");

		}

		i++;
	}
}
