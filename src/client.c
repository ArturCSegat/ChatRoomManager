#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <poll.h>
#include <signal.h>
#include "../headers/network_utils.h"

# define MAX_NAME 20
# define MAX_MSG 200

int running = 1;

void ctrl_c_handler(int si) {
    if (si == SIGINT){
        running = 0;
    }
}

int main(int argc, char *argv[]) {

    signal(SIGINT, ctrl_c_handler);

    char *server_ip = argv[1];

    if (argc != 2) {
        printf("To specify a server's IP Address, please run: %s <server_ip> . Attempting to connect anyways...\n", argv[0]);
        server_ip = NULL;
    }

    char name[MAX_NAME];
    printf("what do you want to be called? :");
    fgets(name, MAX_NAME, stdin);
    __fpurge(stdin); // clear shit from stdin

    name[strcspn(name, "\n")] = 0; 
    
    int sock_fd = get_server_sock_or_die(server_ip);
    send(sock_fd, name, strlen(name), 0);
    memset(name, 0, MAX_NAME);

    printf("\n");
    char msg_buffer[MAX_MSG];
    int msg_b_size = sizeof msg_buffer;

    char recv_msg_buffer[MAX_MSG];
    int recv_b_size = sizeof recv_msg_buffer;
    
    struct pollfd watch_list[1];
    watch_list[0].fd = sock_fd;
    watch_list[0].events = POLLIN;

    if (!fork()) {
        while(running) {
            printf("enter your message: ");
            if (fgets(msg_buffer, msg_b_size, stdin)) {
                int bytes_sent = send(sock_fd, msg_buffer, strlen(msg_buffer), 0);
                memset(msg_buffer, 0, sizeof msg_buffer);
                __fpurge(stdin);
            }
        }
        exit(0);
    }

    while(running) {
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
                    break;
                }
                printf("%s", recv_msg_buffer);
                memset(recv_msg_buffer, 0, sizeof recv_msg_buffer);
                continue;
            }
        }
    }
    close(sock_fd);
    return 0;
}
