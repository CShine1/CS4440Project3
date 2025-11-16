/* This program implements a TCP client that connects to a specified host and port.
 It sends a string to the server, receives the reversed string, and prints it.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define BUF 8192

// main function that sets up the client, connects to the server, sends the string to be reversed, and receives the reversed string
// it uses if and while statements to handle errors during connection, sending, and receiving data
int main(int argc, char **argv) {


    // if statement checks if the correct number of command-line arguments are provided
    if (argc < 4) {
        fprintf(stderr, "Usage: %s <host> <port> <string...>\n", argv[0]);
        return 1;
    }


    char *host = argv[1];
    int port = atoi(argv[2]);

    if (port <= 0) { fprintf(stderr, "Invalid port\n"); return 1; }

    // this part of the code uses a for loop to combine all command-line arguments after the host and port into a single string
    char out[BUF] = {0};
    for (int i = 3; i < argc; ++i) {
        strncat(out, argv[i], sizeof(out) - strlen(out) - 1);
        if (i < argc - 1) strncat(out, " ", sizeof(out) - strlen(out) - 1);
    }

    strncat(out, "\n", sizeof(out) - strlen(out) - 1);

    // if statement creates a TCP socket and checks for errors and the follwinf if statements set up the server address structure and connect to the server
    int s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0) { perror("socket"); return 1; }
    struct sockaddr_in sa;
    memset(&sa, 0, sizeof(sa));
    sa.sin_family = AF_INET;
    sa.sin_port = htons(port);

    if (inet_pton(AF_INET, host, &sa.sin_addr) <= 0) {
        perror("inet_pton");
        close(s);
        return 1;
    }

    if (connect(s, (struct sockaddr*)&sa, sizeof(sa)) < 0) {
        perror("connect");
        close(s);
        return 1;
    }

    ssize_t sent = 0;
    ssize_t need = strlen(out);

    // the while loop sends the constructed string to the server
    // it makes sure the entire string is sent by checking the number of bytes sent against the total
    while (sent < need) {
        ssize_t w = send(s, out + sent, need - sent, 0);
        if (w <= 0) { perror("send"); close(s); return 1; }
        sent += w;
    }

    char in[BUF];
    ssize_t r = recv(s, in, sizeof(in)-1, 0);

    // if statement checks the result of recv to see if data was received, the connection was closed, or an error occurred
    if (r > 0) {
        in[r] = '\0';
        printf("%s", in);
    } else if (r == 0) {
        fprintf(stderr, "Server closed connection\n");
    } else {
        perror("recv");
    }
    close(s);
    return 0;
}