#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

char** tokenize(char* input, char* delimiter);

int main(int argc, char* argv[]){

	int bash_mode_on = 0;

	if(argc == 2){
		
		FILE* bash_fp = fopen(argv[1], "r");
		bash_mode_on = 1;

	}
	else if(argc > 2){
		
		printf("Too many arguments to myshell.\n");
		exit(0);
	}

	char* user_input = NULL;
	size_t size = 0;

	while(1){

		getline(&user_input, &size, stdin);

		printf("user_input id: %p\n", user_input);

		char** token_array = tokenize(user_input, " \t\n");
	
		int i = 0;		

		while(token_array[i] != NULL){
				
			printf("%s\n", token_array[i]);
			i++;
	
		}

		if(strcmp(user_input, "exit\n") == 0){

			printf("Freed user_input: %p\n", user_input);
		
			free(user_input);

			exit(0);
		}
		free(token_array);
	}
	
	printf("Freed user_input: %p\n", user_input);

	free(user_input);
	return 0;
}

char** tokenize(char* user_input, char* delimiter){

	char* user_copy = calloc(strlen(user_input) + 1, sizeof(char));

	strncpy(user_copy, user_input, strlen(user_input) + 1);

	printf("This is the user copy: %s\n", user_copy);

	char** token_array = (char**)malloc(sizeof(char*));

	char* token = strtok(user_copy, delimiter);

	int i = 0;

	while(token != NULL){
		
		token_array[i] = token;
		token = strtok(NULL, delimiter);
		i++;
		token_array = realloc(token_array, (i + 1) * sizeof(char*));
	}

	token_array[i] = NULL;

	return token_array;
}
