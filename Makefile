CC = gcc
CFLAGS = -Wall -Wextra -Werror -lreadline -lm -O2

all:
	@$(CC) -o beval beval.c functions.c $(CFLAGS)

clean:
	@$(RM) beval
