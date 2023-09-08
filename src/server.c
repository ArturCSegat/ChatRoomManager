#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <netdb.h>
#include <poll.h>
#include "../headers/din_arr.h"
#include "../headers/chat_room.h"
#include "../headers/network_utils.h"



#define PORT "6969"
#define MAX_QUEUE 10


// gets the ip of a sockaddr 4 or 6


int main(void) {
    struct addrinfo * sock_info;
    if  ((sock_info = get_sock_info(NULL)) == NULL) {
        printf("error code: %d", errno);
        perror("info");
        freeaddrinfo(sock_info);
        return -1;
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

    printf("listening on %d :...\n", listen_socket);
    if (listen(listen_socket, MAX_QUEUE) == - 1) {
        printf("error: %d\n", errno);
        perror("listen");
        close(listen_socket);
        return -1;
    }
    
    struct chatroom * test_room = chatroom_builder(0, "sala teste");
    add_con(test_room, listen_socket);

    // main accept loop
    while (1) {
        din_arr * senders = new_din_arr(3);
        if (recv_conns(test_room, listen_socket, senders) != 0) {
            printf("error happend when handling new connections\n");
            printf("errno (may not be related: %d", errno);
            perror("may not be related");
        }
        if (senders->len > 0) {
            for (int i = 0; i < senders->len; i++) {
                char msg_buff[200];
                int bytes = recv(senders->arr[i], msg_buff, sizeof msg_buff, 0);
                
                if (bytes <= 0) {
                    close(senders->arr[i]);
                    remove_con(test_room, senders->arr[i]);
                    printf("closed connection from %d\n", senders->arr[i]);
                        sleep(2);
                    continue;
                }
                
                for (int j = 0; j < test_room->fds_len; j++) {
                    if (test_room->fds[j].fd == listen_socket || test_room->fds[j].fd == senders->arr[i]){
                        continue;
                    }
                    if (send(test_room->fds[j].fd, msg_buff, strlen(msg_buff), 0) == - 1) {
                        printf("error on sending to %d\n", test_room->fds[j].fd);
                        perror("send");
                    }
                }

                printf("spreading message %d bytes long from %d (%s)\n", bytes, senders->arr[i], msg_buff);
                memset(msg_buff, 0, sizeof(msg_buff));
            }
        }
        free_din_arr(senders);
    }
    
    free_chat_room(test_room);
}