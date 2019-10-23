#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>

char *get_word(char *end){
	char c = getchar();
	char *array = NULL;
	int array_size = 0;
	while (c != '\n' || c != ' ' || c != '\t' || c != '|'){
		char *temp_array = realloc(array, (++array_size) * sizeof(char));
		if (temp_array == NULL){
			free(array);
			end = '\0';
			return NULL;
		}
		array = temp_array;
		array[array_size - 1] = c;
		c = getchar();
	}
	*end = array[array_size - 1];
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

int main(int argc, char **argv){
	char *cmd_A[3] = {NULL, NULL, NULL}; 
	char *cmd_B[3] = {NULL, NULL, NULL};
	for (int i = 0; i < 2; i++)
	{
		char symbol = '\n';
		while(symbol != '\n' || symbol != ' ')
		cmd_A[i] = get_word(&symbol);
	}
	get_word(NULL); 
	for (int i = 0; i < 2; i++)
	{
		char symbol = '\n';
		while(symbol != '\n' || symbol != ' ' )
		cmd_B[i] = get_word(&symbol);
	}
	pid_t pid1, pid2;
	int s1[2];
	if (pipe(s1) == -1)
	{
		perror("s1 error");
		return 100;
	}
	pid1 = fork();
	if (pid1 < 0){
		perror("pid1");
		return 90;
	}
	if (pid1 == 0){
		dup2(s1[1], 1);
		close(s1[1]);
		close(s1[0]);
		if (execve(cmd_A[0], cmd_A, NULL) < 0){
			perror("execlp error");
			return 80;
		}
	}
	pid2 = fork();
	if (pid2 < 0){
		perror("pid2");
		return 60;
	}
	if (pid2 == 0){
		close(s1[0]);
		close(s1[1]);
		if (execve(cmd_B[0], cmd_B, NULL) < 0){
			perror("execlp error");
			return 50;
		}
	}
	close(s1[0]);
	close(s1[1]);
	int flag = 0;
	for (int i = 0; i < 3; ++i){
		int status;
		wait(&status);
		if(!(WIFEXITED(status) && WEXITSTATUS(status) == 0)){
			flag++;
		}
	}
	if (flag){
		return 20;
	}
	else{
		return 0;
	}
}