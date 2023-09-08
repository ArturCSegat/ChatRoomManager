#include <sys/poll.h>
#include "network_utils.h"

#define PORT "6969"


int main(void) {
    
    int sock_fd = get_server_sock_or_die();

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
                    break;
                }
                printf("\nmessage from server: %s\n", recv_msg_buffer);
                memset(recv_msg_buffer, 0, sizeof recv_msg_buffer);
                continue;
            }
        }
    }
    printf("end\n");
    close(sock_fd);
    return 0;
}
