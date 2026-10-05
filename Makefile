CC = gcc
CFLAGS = -Wall -g -Ilist

all: serv cli

serv: serv.c list/list.c
	$(CC) $(CFLAGS) serv.c list/list.c -o server

cli: cli.c
	$(CC) $(CFLAGS) cli.c -o client

clean:
	rm -f server client