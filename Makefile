CC = gcc
CFLAGS = -Wall -Wextra -Werror -lreadline -lm -O2 -D_version=\"`git describe --tags --abbrev=0`\"

all:
	@$(CC) -o beval beval.c functions.c $(CFLAGS)

clean:
	@$(RM) beval
