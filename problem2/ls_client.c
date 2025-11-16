/* This program implements a TCP client that connects to a directory-listing server.
It sends "ls" command arguments constructed from command-line arguments to the server,
then receives and prints the directory listing output sent back by the server.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define BUF 8192

// main function that sets up the client, connects to the server, sends the "ls" command arguments, and receives the output
// it also uses if and while statements to handle errors during connection, sending, and receiving data
int main(int argc, char **argv) {
    if (argc < 4) {
        fprintf(stderr, "Usage: %s <host> <port> <ls args...>\n", argv[0]);
        return 1;
    }
    char *host = argv[1];
    int port = atoi(argv[2]);
    if (port <= 0) { fprintf(stderr, "Invalid port\n"); return 1; }

    int s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0) { perror("socket"); return 1; }

    // this section of our code sets up the server address structure and converts the host IP address
    struct sockaddr_in sa;
    memset(&sa, 0, sizeof(sa));
    sa.sin_family = AF_INET;
    sa.sin_port = htons(port);
    if (inet_pton(AF_INET, host, &sa.sin_addr) <= 0) {
        perror("inet_pton");
        close(s);
        return 1;
    }
    // this section of our code connects to the server
    if (connect(s, (struct sockaddr*)&sa, sizeof(sa)) < 0) {
        perror("connect");
        close(s);
        return 1;
    }

    // this section of our code constructs the payload to send to the server
    // it concatenates all command-line arguments after the host and port into a single string
    // the for loop iterates through the arguments and appends them to the payload string and the if statement adds spaces between arguments
    char payload[BUF];
    payload[0] = '\0';
    for (int i = 3; i < argc; ++i) {
        strncat(payload, argv[i], sizeof(payload) - strlen(payload) - 2);
        if (i < argc - 1) strncat(payload, " ", sizeof(payload) - strlen(payload) - 2);
    }
    strncat(payload, "\n", sizeof(payload) - strlen(payload) - 1);

    // the while loop sends the constructed payload to the server
    // it makes sure the entire payload is sent by checking the number of bytes sent against the total
    ssize_t need = strlen(payload);
    ssize_t sent = 0;
    while (sent < need) {
        ssize_t w = send(s, payload + sent, need - sent, 0);
        if (w <= 0) { perror("send"); close(s); return 1; }
        sent += w;
    }

    char buf[BUF];
    ssize_t r;
    while ((r = recv(s, buf, sizeof(buf), 0)) > 0) {
        fwrite(buf, 1, r, stdout);
    }
    if (r < 0) perror("recv");

    close(s);
    return 0;
}