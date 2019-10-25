#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>

int error_malloc = 0;
int arg_c = 0;

struct node
{
	char *el;
	struct node *next;
};

typedef struct node Node;
typedef Node* Link;

Link pushfront(Link headptr, char **word){
	Link ptr = malloc(sizeof(Node));
	if (ptr == NULL){
		perror("malloc error");
		return NULL;
	}
	ptr -> el = *word;
	ptr -> next = headptr;
	return ptr;
}

Link pushback(Link headptr, char **word){
	if (headptr == NULL){
		return pushfront(headptr, word);
	}
	Link ptr = headptr;
	while (ptr -> next){
		ptr = ptr -> next;
	}
	ptr -> next = pushfront(NULL, word);
	return headptr;
}

Link deletelink(Link *headptr, char *word)
{
	if (!(*headptr)){
		perror("Link is clear");
		return NULL;
	}
	if (!strcmp((*headptr) -> el, word)){
		Link t = *headptr;
		*headptr = t -> next;
		return t;
	}
	Link t = *headptr;
	while (t -> next && strcmp(t -> next -> el, word)){
		t = t -> next;
	}
	if (!(t -> next) || !strcmp(t -> next -> el, word)){
		perror("word is not found in Link");
		return NULL;
	}
	Link q = t -> next;
	t -> next = q -> next;
	return q;
}

Link clear(Link *head) 
{
	Link ptr;
	while (*head != NULL) 
	{
		ptr = (*head) -> next;
		free((*head));
		(*head) = ptr;
	}
	return NULL;
}

char *get_word(char *end){
	char c = getchar();
	char *array = NULL;
	int array_size = 0;
	int flg = 0; // этот флаг нужен для проверок ковычек
	if (c == '"'){
		flg = 1;
		c = getchar();
	}
	while (c != '\n' && ((c != ' ' && c != '\t' && c != '|') || flg)){	//всякие проверки, но окончанием команды всегда будет ENTER!!!
		if (c == '"'){
			c = getchar();
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
		c = getchar();
	}
	if (flg){	//если мы не встретили закрывающтеся ковычки, то не позволяем пользователю вводить что-то дальше
		*end = '\0';
		return NULL;
	}
	*end = c;
	if (array == NULL){
		return NULL;
	}
	char *temp_array = realloc(array, (++array_size) * sizeof(char));
	if (temp_array == NULL){
		perror("realloc error");
		free(array);
		*end = '\0';
		return NULL;
	}
	array = temp_array;
	array[array_size - 1] = '\0';	// окончание слово будет символ 
	return array;
}

Link get_list(){
	arg_c = 0;
	char *word = NULL, symbol = '\0';
	Link list = NULL;
	while (symbol != '\n'){
		word = get_word(&symbol);
		if (word == NULL && symbol == '\0'){
			clear(&list);
			return NULL;
		} else if (word == NULL && (symbol == '\n' || symbol == '|')){
			break;
		} else if (word == NULL && (symbol == ' ' || symbol == '\t')){
			continue;
		}
		list = pushback(list, &word);
		arg_c++;		
	}
	list = pushback(list, NULL);
	arg_c++;
	return list;
}

int check_right(Link list){
	if (list == NULL || error_malloc){
		return -1;
	}
	if (!strcmp(list -> el, "exit") || !strcmp(list -> el, "quit")){
		return 1;
	} 
	return 0;
}

int mk_chld_proc(Link list){
	pid_t child = fork();
	if (child == -1){
		perror("fork failed");
		return -1;
	} else if (child == 0){
		char **temp_array = malloc(arg_c * sizeof(char *));
		for (int i = 0; i < arg_c; ++i)
		{
			temp_array[i] = list -> el;
			list = list -> next;
		}
		if (execvp(temp_array[0], temp_array) < 0) {
			perror("exec failed");
			return -1;
		}
	}
	wait(NULL);
	return 0;
}

int main(){
	while (1){
		Link list = get_list();
		if (check_right(list) == 1){
			break;
		} else if (check_right(list) == -1){
			clear(&list);
			continue;
		}
		mk_chld_proc(list);
		clear(&list);
	}
	return 0;
}