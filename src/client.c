#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <poll.h>
#include "../headers/network_utils.h"



int main(int argc, char *argv[]) {

    char *server_ip = argv[1];

    if (argc != 2) {
        printf("To specify a server's IP Address, please run: %s <server_ip> . Attempting to connect anyways...\n", argv[0]);
        server_ip = NULL;
    }

    char name[20];
    printf("what do you want to be called? :");
    fgets(name, sizeof name, stdin);
    name[strcspn(name, "\n")] = 0; 
    
    printf("sendind: %s\n", name);

    int sock_fd = get_server_sock_or_die(server_ip);
    send(sock_fd, name, strlen(name), 0);

    printf("\n");
    char msg_buffer[200];
    int msg_b_size = sizeof msg_buffer;

    char recv_msg_buffer[200];
    int recv_b_size = sizeof recv_msg_buffer;
    
    struct pollfd watch_list[1];
    watch_list[0].fd = sock_fd;
    watch_list[0].events = POLLIN;

    // fcntl(0, F_SETFL, fcntl(0, F_GETFL) | O_NONBLOCK);
    if (!fork()) {
        while(1) {
            printf("enter your message: ");
            if (fgets(msg_buffer, msg_b_size, stdin)) {
                int bytes_sent = send(sock_fd, msg_buffer, strlen(msg_buffer), 0);
                memset(msg_buffer, 0, sizeof msg_buffer);
            }
        }
        printf("end\n");
    }

    while(1) {
        int to_recv = poll(watch_list, 1, 1000);
        
        if (to_recv) {
            if (watch_list[0].revents & POLLHUP) {
                printf("\nThe server closed connection\n");
                break;
            }

            if (watch_list[0].revents & POLLIN) {
                int bytes_received = recv(sock_fd, recv_msg_buffer, recv_b_size, 0);
                if (bytes_received <= 0) {
                    printf("\nThe server closed connection\n");
                    exit(1);
                    break;
                }
                printf("%s", recv_msg_buffer);
                memset(recv_msg_buffer, 0, sizeof recv_msg_buffer);
                continue;
            }
        }
    }
    printf("end\n");
    close(sock_fd);
    return 0;
}
