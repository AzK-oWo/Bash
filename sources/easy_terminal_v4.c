#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

int readfd = -1, writefd = -1;		//если открываем файл
int readflag = 0, writeflag = 0;		//проверка на открытие файлов, проверка на ошибки

void read_file(char *word){ //если открывается 2ой файл на чтение то игнорит его
	if (readfd == -1 && readflag == 0){
		readfd = open(word, O_RDONLY);
		if (readfd < 0){
			readflag = 0;
			return;
		}
		readflag++;
	}
}

void write_file(char *word){	//если открывает 2ой файл на запись, то игнорит его
	if (writefd == -1 && writeflag == 0){
		writefd = open(word, O_CREAT | O_WRONLY | O_TRUNC, 0664);
		if (writefd < 0){
			writeflag = 0;
			return;
		}
		writeflag++;	
	}
}

char *get_word(char *end){
	char c;
	if (read(0, &c, sizeof(char)) < 0){
		perror("word was not read");
		*end  = '\0';
		return NULL;
	}	
	if (c == '>' || c == '<'){	//проверка на открытие файла
		*end = c;
		return NULL;
	}
	char *array = NULL;
	int array_size = 0;
	int flg = 0;	// этот флаг нужен для проверок ковычек
	if (c == '"'){
		flg = 1;	
		if (read(0, &c, sizeof(char)) < 0){
			perror("word was not read");
			*end  = '\0';
			return NULL;
		}	
	}
	while (c != '\n' && ((c != ' ' && c != '\t' && c != '|') || flg)){	//всякие проверки, но окончанием команды всегда будет ENTER!!!
		if (c == '"'){
			if (read(0, &c, sizeof(char)) < 0){
				perror("word was not read");
				*end  = '\0';
				return NULL;
			}	
			if(c != ' ' && c != '\n' && c != '|' && c != '\t'){	//если после ковычек не идет SPACE, TAB, PIPE или ENTER
				*end = '\0';
				return NULL;
			}
			flg = 0;
			break;
		}
		char *temp_array = realloc(array, (++array_size) * sizeof(char));
		if (temp_array == NULL){
			perror("realloc error");
			free(array);
			*end = '\0';
			return NULL;
		}
		array = temp_array;
		array[array_size - 1] = c;
		if (read(0, &c, sizeof(char)) < 0){
			perror("word was not read");
			*end  = '\0';
			return NULL;
		}	
	}
	if (flg){	//если мы не встретили закрывающиеся ковычки, то не позволяем пользователю вводить что-то дальше
		*end = '\0';
		return NULL;
	}
	*end = c;
	if (array == NULL){
		return NULL;
	}
	char *temp_array = realloc(array, (++array_size) * sizeof(char)); // выделяем память на окончание слова
	if (temp_array == NULL){
		perror("realloc error");
		free(array);
		*end = '\0';
		return NULL;
	}
	array = temp_array;
	array[array_size - 1] = '\0';	// окончание словa будет символ 
	return array;
}

char **get_list(char *symbol){
	int arg_c = 0;
	char **list = NULL, *word = NULL; 
	*symbol = '\0';
	while (*symbol != '\n' && *symbol != '|'){ // проверка на окончание команды или pipe
		word = get_word(&(*symbol));
		if (*symbol == '<' || *symbol == '>'){ // проверка на открытие файла
			char tmp = *symbol; // сохраняем нашу переменную
			word = get_word(&(*symbol)); // ищем наш файл, который нужно прочитать/ на который нужно записать
			while (word == NULL && (*symbol != '\n' && *symbol != '|')){
				if (word == NULL && *symbol == '\0'){ //если возникла ошибка при поиске имени файла, то завершаем
					for (int i = 0; i < arg_c; i++){
						free(list[i]);
					}
					free(list);
					if (writeflag){
						close(writefd);
						writeflag = 0;
					}
					if (readflag){
						close(readfd);
						readflag = 0;
					}
					return NULL;
				}
				word = get_word(&(*symbol));	// продолжаем искать
			}
			if (word == NULL && (*symbol == '\n' || *symbol == '|')){	//если после > или < вдруг нажали на ENTER или поставили PIPE
				perror("name of file not found");
				for (int i = 0; i < arg_c; i++){
					free(list[i]);
				}
				free(list);
				*symbol = '\0';
				if (writeflag){
					close(writefd);
					writeflag = 0;
				}
				if (readflag){
					close(readfd);
					readflag = 0;
				}
				return NULL;
			}
			if (tmp == '<'){
				read_file(word);
				if (readfd < 0 && readflag == 0){ // проверка, если файл не открылся на чтение
					perror("file did not open");
					*symbol = '\0';
					for (int i = 0; i < arg_c; i++){
						free(list[i]);
					}
					free(list);
					return NULL;
				}
			} else {
				write_file(word);
				if (writeflag == 0 && writefd < 0){ // проверка, если файл не открылся на запись
					perror("file did not open");
					*symbol = '\0';
					for (int i = 0; i < arg_c; i++){
						free(list[i]);
					}
					free(list);
					return NULL;
				}
			}
			free(word);
			word = NULL;
		}
		if (word == NULL && *symbol == '\0'){ // проверка на ошибку в get_word
			for (int i = 0; i < arg_c; i++){
				free(list[i]);
			}
			free(list);
			if (writeflag){
				close(writefd);
				writeflag = 0;
			}
			if (readflag){
				close(readfd);
				readflag = 0;
			}
			return NULL;
		} else if (word == NULL && (*symbol == '\n' || *symbol == '|')){ // проверка на окончание команды, например: "ls\t\n" или "ls |"
			break;
		} else if (word == NULL && (*symbol == ' ' || *symbol == '\t')){ // проверка на SPACE или TAB
			continue;
		}
		char **temp_array = realloc(list, (arg_c + 1) * sizeof(char *));
		if (temp_array == NULL){
			perror("realloc error");
			for (int i = 0; i < arg_c; i++){
				free(list[i]);
			}
			free(list);
			free(word);
			if (writeflag){
				close(writefd);
				writeflag = 0;
			}
			if (readflag){
				close(readfd);
				readflag = 0;
			}
			*symbol = '\0';
			return NULL;
		}
		list = temp_array;
		list[arg_c] = word;
		arg_c++;
	}
	char **temp_array = realloc(list, (arg_c + 1) * sizeof(char *)); //выделяем указатель на NULL
	if (temp_array == NULL){
		perror("realloc error");
		for (int i = 0; i < arg_c; i++){
			free(list[i]);
		}
		free(list);
		if (writeflag){
			close(writefd);
			writeflag = 0;
		}
		if (readflag){
			close(readfd);
			readflag = 0;
		}
		*symbol = '\0';
		return NULL;
	}
	list = temp_array;
	list[arg_c] = NULL; //заканчиваем наш "список" NULL
	return list;
}

void clear_list(char ***list){ // очистка массива указателей на предложение
	for(int i = 0; ;i++){
		if ((*list)[i] == NULL){
			free((*list)[i]);
			break;
		}
		free((*list)[i]);
	}
	free(*list);
}

void clear_cmd(char ****cmd, int arg_c){ // очистка массива указателей на предложения
	for(int i = 0; i < arg_c; i++){
		clear_list(&((*cmd)[i]));
	}
	free(*cmd);
}

char ***get_cmd(int *arg_c, int *err){
	*err = 0; //err == 1 - выход из программы, err == 0 - нормальное считывание, -err == -1 - считывание с ошибкой или пустой список
	*arg_c = 0;
	char symbol;
	char **list = get_list(&symbol);
	if (list == NULL){
		*err = -1;
		return NULL;
	}
	if (!strcmp(list[0], "exit") || !strcmp(list[0], "quit")){ //  проверка на завершения программы
		clear_list(&list);
		*err = 1;
		return NULL;
	}
	char ***cmd = malloc(sizeof(char **));
	if (cmd == NULL){
		perror("malloc error");
		free(list);
		clear_cmd(&cmd, *arg_c);
		*err = -1;
		return NULL;
	}
	cmd[0] = list;
	(*arg_c)++;
	while (symbol != '\n'){
		list = get_list(&symbol);
		if (list == NULL && symbol == '\0'){
			clear_cmd(&cmd, *arg_c);
			*err = 1;
			return NULL;
		}
		char ***temp_array = realloc(cmd, ((*arg_c) + 1) * sizeof(char **));
		if (temp_array == NULL){
			perror("realloc error");
			free(list);
			*err = -1;
			clear_cmd(&cmd, *arg_c);
			return NULL;
		}
		cmd = temp_array;
		cmd[*arg_c] = list;
		(*arg_c)++;
	}
	return cmd;
}

int mk_pipeline(int (*ppe)[2], int num){
	ppe = malloc(num * sizeof(int *));
	if (ppe == NULL){
		perror("pipe did not creat");
		return -1;
	}
	for (int i = 0; i < num; ++i){
		if (pipe(ppe[i]) == -1){
			for (int j = 0; j < i; j++){
			close(ppe[i][0]);
			close(ppe[i][1]);
			free(ppe[i]);
			}
			return -1;
		}
	}	
	return 0;
}

void mk_chld_proc(char ***cmd, int arg_c){
	pid_t child;
	int (*ppe)[2] = NULL;
	int tmp = mk_pipeline(ppe, arg_c - 1);
	for (int i = 0; i < arg_c; i++){
		if(ppe[i] != NULL){
			printf("%d ppe\n", i);
		}
	}
	if (tmp == -1){
		return;
	}
	for(int i = 0; i < arg_c; i++){
		child = fork();
		if (child == -1){
			perror("fork failed");
			return;
		} else if (child == 0){
			if (i == 0 && readflag == 1){
				dup2(readfd, 0);
				close(readfd);
			}
			if (ppe != NULL){
				if (i != arg_c - 1){
					dup2(ppe[i][1], 1);
				}
				if (i != 0){
					dup2(ppe[i - 1][0], 0);
				}
			}
			for (int j = 0; j < arg_c - 1; j++){
				close(ppe[j][0]);
				close(ppe[j][1]);
			}
			if (i == arg_c - 1 && writeflag == 1){
				dup2(writefd, 1);
				close(writefd);
			}
			if (execvp(cmd[i][0], cmd[i]) < 0){
				if (writeflag){
					close(writefd);
				}
				if (readflag){
					close(readfd);
				}
				perror("exec failed");
				exit(1);
			}
		} else {
			wait(NULL);
		}
	}
	for (int i = 0; i < arg_c - 1; ++i)
	{
		close(ppe[i][0]);
		close(ppe[i][1]);
		free(ppe[i]);
	}
	return;
}

void back_to_begin(char ****cmd, int arg_c){
	clear_cmd(cmd, arg_c);
	if (writeflag){
		close(writefd);
	}
	if (readflag){
		close(readfd);
	}
	writefd = -1;
	readfd = -1;
	writeflag = 0;
	readflag = 0;
}

int main(){
	while (1){
		int err = 0, arg_c = 0;
		char ***cmd = get_cmd(&arg_c, &err);
		if (err == 1){
			break;
		} else if (err == -1){
			continue;
		}
		mk_chld_proc(cmd, arg_c);
		back_to_begin(&cmd, arg_c);
	}
	return 0;
}