%: %.c
	gcc $@.c -o $@ -Wall -Werror -lm
