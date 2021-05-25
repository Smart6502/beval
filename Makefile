CC = gcc
CFLAGS = -Wall -Wextra -Werror -lreadline -O2 -g # just for debugging

all:
	@$(CC) -o beval beval.c $(CFLAGS)

clean:
	@$(RM) beval
