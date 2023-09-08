#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <errno.h>
#include <netdb.h>
#include "../network_utils.h"

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

    if (getaddrinfo(NULL, PORT, &hints, &socket_info) != 0) {
        return NULL;
    }
    return socket_info;
}

int get_server_sock_or_die() {
    struct addrinfo * sock_info;   
    if  ((sock_info = get_sock_info(NULL)) == NULL) {
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
