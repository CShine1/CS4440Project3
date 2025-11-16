# Project 3 Makefile

CC = gcc
CFLAGS = -Wall -pthread

# build everything
all: server client ls_server ls_client

# problem 1 - basic client-server
server: server.c
	$(CC) $(CFLAGS) server.c -o server

client: client.c
	$(CC) $(CFLAGS) client.c -o client

# problem 2 - directory listing server
ls_server: ls_server.c
	$(CC) $(CFLAGS) ls_server.c -o ls_server

ls_client: ls_client.c
	$(CC) $(CFLAGS) ls_client.c -o ls_client


# problem 3 - basic disk-storage system


# problem 4 - file system server


#tidy
clean:
	rm -f server client ls_server ls_client

.PHONY: all clean

