/* This program implements a TCP server that listens on a specified port for incoming connections.
It accepts the connection and makes a new thread to handle the client server.
The string it gets from tthe client the server reveres and returns to the client.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define BACKLOG 16
#define BUF_SIZE 4096

// this functions prints out an error message and exists the program if a fatal error occurs
static void fatal(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

// typedef struct to hold client connection information for the thread
typedef struct {
    int client_fd;
    struct sockaddr_in addr;
} client_arg_t;

/// static function to reverse a string in place
static void reverse_inplace(char *s, ssize_t n) {
    ssize_t i = 0, j = n - 1;
    while (i < j) {
        char t = s[i];
        s[i] = s[j];
        s[j] = t;
        ++i; --j;
    }
}

/// static function that serves as the main function for each client-handling thread
static void *thread_main(void *arg) {
    client_arg_t *ca = (client_arg_t *)arg;
    int fd = ca->client_fd;
    free(ca);
    
    // prints thread started message with thread ID and client file descriptor on the console every time a new thread is created/ client connects
    // the sleep is there to simulate denial of service, when overwhelmed the server stops responding
    // it slows the server down to show how too many connections can affect performance
    printf("Thread %lu started for client fd=%d\n", pthread_self(), fd);
    sleep(2);

    // receives data from the client, reverses the string, and sends it back
    char buf[BUF_SIZE];
    ssize_t r = recv(fd, buf, sizeof(buf)-1, 0);
    if (r <= 0) {
        close(fd);
        return NULL;
    }

    // if statement checks if the last character received is a newline and removes it before reversing
    // buf[r] makes sure the string is null-terminated before reversing
    if (buf[r-1] == '\n') {
        --r;
    }
    buf[r] = '\0';
    reverse_inplace(buf, r);
    

    // the strncat adds a newline character to the end of the reversed string
    // while loop sends the reversed string back to the client
    strncat(buf, "\n", sizeof(buf) - strlen(buf) - 1);
    ssize_t tosend = strlen(buf);
    ssize_t sent = 0;
    while (sent < tosend) {
        ssize_t s = send(fd, buf + sent, tosend - sent, 0);
        if (s <= 0) break;
        sent += s;
    }
    close(fd);
    return NULL;
}

// main function that sets up the server, listens for incoming connections, and creates threads to handle each client
// it uses if and while statements to handle errors during socket creation, binding, listening, and accepting connections
int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        return 1;
    }
    int port = atoi(argv[1]);
    if (port <= 0) {
        fprintf(stderr, "Invalid port\n");
        return 1;
    }

    int srv = socket(AF_INET, SOCK_STREAM, 0);
    if (srv < 0) fatal("socket");
    int opt = 1;
    if (setsockopt(srv, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        fatal("setsockopt");
    }

    struct sockaddr_in sa;
    memset(&sa, 0, sizeof(sa));
    sa.sin_family = AF_INET;
    sa.sin_addr.s_addr = INADDR_ANY;
    sa.sin_port = htons(port);

    if (bind(srv, (struct sockaddr*)&sa, sizeof(sa)) < 0) fatal("bind");
    if (listen(srv, BACKLOG) < 0) fatal("listen");

    printf("server: listening on port %d\n", port);

    //while loop that accepts connections and creates a new thread for each client
    while (1) {
        client_arg_t *ca = malloc(sizeof(client_arg_t));
        if (!ca) fatal("malloc");
        socklen_t addrlen = sizeof(ca->addr);
        ca->client_fd = accept(srv, (struct sockaddr*)&ca->addr, &addrlen);
        if (ca->client_fd < 0) {
            perror("accept");
            free(ca);
            continue;
        }
        pthread_t tid;
        if (pthread_create(&tid, NULL, thread_main, ca) != 0) {
            perror("pthread_create");
            close(ca->client_fd);
            free(ca);
            continue;
        }
        pthread_detach(tid);
    }

    close(srv);
    return 0;
}
