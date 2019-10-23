#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>

char *get_word(char *end){
	char c = getchar();
	char *array = NULL;
	int array_size = 0;
	while (c != '\n' && c != ' ' && c != '\t'){
		array = realloc(array, (++array_size) * sizeof(char));
		array[array_size - 1] = c;
		c = getchar();
	}
	*end = c;
	if (array == NULL){
		return NULL;
	}
	array = realloc(array, (++array_size) * sizeof(char));
	array[array_size - 1] = '\0';
	return array;
}

char **get_list(){
	int arg_c = 0;
	char **list = NULL, *word = NULL, symbol = '\0';
	while (symbol != '\n'){// read commands
		word = get_word(&symbol);
		if (word == NULL && symbol == '\n'){
			break;
		} else if (word == NULL && symbol == ' '){
			continue;
		}
		list = realloc(list, (arg_c + 1) * sizeof(char *));
		list[arg_c] = word;
		arg_c++;
	}
	list = realloc(list, (arg_c + 1) * sizeof(char *));
	list[arg_c] = NULL;
	return list;
}

void clear(char ***list){
	char **tmp = *list;
	for(int i = 0; ; i++){
		if (tmp[i] == NULL){
			free(tmp[i]);
			break;
		}
		free(tmp[i]);
	}
}
int check_right(char **list){
	if (list == 0){
		return -1;
	}
	if (!strcmp(list[0], "exit") || !strcmp(list[0], "quit")){
		return 1;
	} 
	return 0;
}

int mk_chld_proc(char **list){
	pid_t child = fork();
	if (child == -1){
		perror("fork failed");
		return -1;
	} else if (child == 0){
		if (execvp(list[0], list) < 0) {
			perror("exec failed");
			return -1;
		}
	}
	wait(NULL);
	return 0;
}

int main(){
	while (1){
		char **list = get_list();
		if (check_right(list) == 1){
			break;
		} else if (check_right(list) == -1){
			continue;
		}
		mk_chld_proc(list);
		clear(&list);
	}
	return 0;
}