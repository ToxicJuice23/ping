#include <stdio.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

char* get_url(char* buf) {
    char* start = strstr(buf, " ");
    if (!start) {
        return (char*)0;
    }
    start++;
    char* end = strstr(start, " ");
    if (!end)
        return NULL;
    *end = 0;

    int len = end - start;
    char* res = malloc(len);
    memcpy(res, start, len);
    res[len] = 0;
    return res;
}

// the goal of this pgm is to remotely connect and make a sound when a button is pressed so you can find your device
int main(int argc, char** argv) {
    if (argc < 2 || argc > 2) {
        printf("Usage: ./main <port>\n");
        exit(1);
    }
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    struct addrinfo* bindaddr;
    int res = getaddrinfo(0, argv[1], &hints, &bindaddr);
    if (res) {
        printf("Getaddrinfo failed. %d\n", errno);
        exit(1);
    }

    int socket_listen = socket(bindaddr->ai_family, bindaddr->ai_socktype, bindaddr->ai_protocol);
    res = bind(socket_listen, bindaddr->ai_addr, bindaddr->ai_addrlen);
    if (res) {
        printf("Binding failed. %d\n", errno);
        exit(1);
    }

    printf("Listening.\n");
    listen(socket_listen, 10);

    struct sockaddr* clientaddr = malloc(sizeof(struct sockaddr*));
    socklen_t clientlen = (socklen_t)sizeof(clientaddr);

    while (1) {
        int client = accept(socket_listen, clientaddr, &clientlen);
        printf("Client connected.\n");

        char buf[1024];
        int br = (int)recv(client, buf, 1024, 0);
        printf("Received %d bytes.\n", br);
        sprintf(buf, "%.*s", br, buf);

        char* path = get_url(buf);
        if (path) {
            if (strcmp(path, "/") == 0) {
                char *msg = "HTTP/1.1 200 OK\r\n"
                            "Connection: close\r\n"
                            "Content-Type: text/html\r\n\r\n"
                            "<!DOCTYPE html><html>"
                            "<body>"
                            "<a href='/ping'><button><h1>PING</h1></button></a>"
                            "</body>"
                            "</html>";
                send(client, msg, strlen(msg), 0);
                close(client);
            } else if (strcmp(path, "/ping") == 0) {
                char *msg = "HTTP/1.1 200 OK\r\n"
                            "Connection: close\r\n"
                            "Content-Type: text/html\r\n\r\n"
                            "<!DOCTYPE html><html>"
                            "<body>"
                            "<a href='/ping'><button><h1>PING</h1></button></a>"
                            "</body>"
                            "</html>";
                send(client, msg, strlen(msg), 0);
                close(client);
                system("aplay ping.wav");
            } else {
                char *msg = "HTTP/1.1 404 Not Found\r\n"
                            "Connection: close\r\n"
                            "Content-Type: text/html\r\n\r\n"
                            "<!DOCTYPE html><html>"
                            "<body>"
                            "<h2>404 Not Found</h2>"
                            "</body>"
                            "</html>";
                send(client, msg, strlen(msg), 0);
                close(client);
            }
        } else {
            close(client);
        }
    }
}
