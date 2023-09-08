#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <errno.h>
#include <netdb.h>
#include "../network_utils.h"

#define PORT "6969"
#define MAX_QUEUE 10

void *get_in_addr(struct sockaddr *sa) {

    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

struct addrinfo * get_sock_info(const char * ip) {
    struct addrinfo hints;
    struct addrinfo * socket_info;
    
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    if (ip == NULL) {
        hints.ai_flags = AI_PASSIVE;
    }

    if (getaddrinfo(ip, PORT, &hints, &socket_info) != 0) {
        return NULL;
    }
    return socket_info;
}

int get_server_sock_or_die(const char *server_ip) {
    struct addrinfo *sock_info;

    if ((sock_info = get_sock_info(server_ip)) == NULL) {
        printf("error code: %d", errno);
        perror("info");
        freeaddrinfo(sock_info);
        exit(1);
    }

    int sock_fd;
    struct addrinfo * p;
    for (p = sock_info; p != NULL; p = p->ai_next) {
        if ((sock_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == - 1) {
            printf("error code: %d", errno);
            perror("socket");
            continue;
        } 
        
        if (connect(sock_fd, p->ai_addr, p->ai_addrlen) == - 1) {
            close(sock_fd);
            printf("error code: %d", errno);
            perror("connect");
            continue;
        }
        
        break;
    }

    if (p == NULL) {
        printf("errno: %d\n", errno);
        printf("couldn't connect\n");
        perror("connect error: ");
        freeaddrinfo(sock_info);
        exit(1);
    }
    
    freeaddrinfo(sock_info);
    return sock_fd;
}

int get_listening_sock_or_die() {
    struct addrinfo * sock_info;
    if  ((sock_info = get_sock_info(NULL)) == NULL) {
        printf("error code: %d", errno);
        perror("info");
        freeaddrinfo(sock_info);
        exit(1);
    } 

    int listen_socket;
    struct addrinfo * p;
    for (p = sock_info; p != NULL; p = p->ai_next) { 
        if ((listen_socket = socket(sock_info->ai_family, sock_info->ai_socktype, sock_info->ai_protocol)) == - 1) {
            printf("error: %d\n", errno);
            perror("sock");
            continue;
        }

        if (bind(listen_socket, sock_info->ai_addr, sock_info->ai_addrlen) == - 1) {
            printf("error: %d\n", errno);
            perror("bind");
            close(listen_socket);
            continue;
        }

        break;
    }
    freeaddrinfo(sock_info);

    if (p == NULL)  {
        fprintf(stderr, "server: failed to bind\n");
        printf("errno: %d\n", errno);
        perror("bind fail");
        exit(1);
    }

    if (listen(listen_socket, MAX_QUEUE) == - 1) {
        printf("error: %d\n", errno);
        perror("listen");
        close(listen_socket);
        exit(1);
    }

    return listen_socket;
}
