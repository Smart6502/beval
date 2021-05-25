CC = gcc
CFLAGS = -Wall -Wextra -Werror -lreadline -O2

all:
	@$(CC) -o beval beval.c $(CFLAGS)

clean:
	@$(RM) beval
