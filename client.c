#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>

#define PORT "6969"

struct addrinfo * get_sock_info() {
    struct addrinfo hints;
    struct addrinfo * socket_info;
    
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if (getaddrinfo(NULL, PORT, &hints, &socket_info) != 0) {
        return NULL;
    }
    return socket_info;
}

void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(void) {
    struct addrinfo * sock_info;   
    if  ((sock_info = get_sock_info()) == NULL) {
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
        exit(1);
    }


    char msg_buffer[200];
    int msg_b_size = sizeof msg_buffer;

    if (fgets(msg_buffer, msg_b_size, stdin)) {
        int bytes_sent = send(sock_fd, msg_buffer, msg_b_size, 0);
        char con [INET6_ADDRSTRLEN];
        struct sockaddr host;
        getpeername(sock_fd, &host, (socklen_t *)sizeof host);
        inet_ntop(host.sa_family, get_in_addr(&host), con, sizeof(con));

        printf("sent %d bytes (%s) to %s\n", bytes_sent, msg_buffer, con);
    }
    
    close(sock_fd);
    return 0;
}
