#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

char *getword(char *end){
	char c = getchar();
	char *array = NULL;
	int array_size = 0;
	while(c == ' ' || c == '\t'){
		c = getchar();
	}
	while (c != '\n' || c != ' ' || c != "\t"){
		char *temp_array = realloc(array, (++array_size) * sizeof(char));
		if (temp_array == NULL){
			free(array);
			end = '\0'
			return NULL;
		}
		array = temp_array;
		array[array_size - 1] = c;
		c = getchar();
	}
	end = array[temp_array - 1];
	char *temp_array = realloc(array, (++array_size) * sizeof(char));
	if (temp_array == NULL){
		free(array);
		end = '\0';
		return NULL;
	}
	array = temp_array;
	array[array_size - 1] = '\0';
	return array;
}

void clear(char ***list){
	for (int i = 0; i < arg_c; i++){
		free(list[i]);
	}
	free(list);
} 

char **get_list(){
	int arg_c = 0, do_nthg_flag = 0, err_flag = 0;
	char **list = NULL, *word = NULL, symbol;
	while (1){// read commands
		word = get_word(&symbol);
		if (word == NULL){
			if (symbol == '\0'){	
				err_flag++;
				break;
			}
			else{
				do_nthg_flag++;
				break;
			}
		}
		temp_list = realloc(list, (arg_c + 1) * sizeof(char *))
		if (temp_list == NULL){
			err_flag++;
			clear(&list);
			break;
		} 
		list = temp_list;
		list[arg_c] = word;
		arg_c++;
		if (symbol == '\n'){
			break;
		}
	}
	return list;
}

int main(){
	pid_t child;
	while (1){
		char **list = get_list();
		child = fork();
		if (child == -1){
			clear(list);
			continue;
		}else if (child == 0){
			if ( execvp(list[0], list) < 0) {
				perror("exec failed");
				clear(&list);
				return 1;
			}
		}
	}
	return 0;
}