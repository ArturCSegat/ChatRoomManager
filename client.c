#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/poll.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <poll.h>
#include <fcntl.h>
#include <ncurses.h>

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


    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);

    int input_height = 3; // Adjust the height as needed
    WINDOW *input_win = newwin(input_height, COLS, LINES - input_height, 0);
    scrollok(input_win, TRUE);

    refresh();

    char msg_buffer[200];
    int msg_b_size = sizeof msg_buffer;
    char recv_msg_buffer[200];
    int recv_b_size = sizeof recv_msg_buffer;
    struct pollfd watch_list[1];
    watch_list[0].fd = sock_fd;
    watch_list[0].events = POLLIN;
    
    if (!fork()) {
        while (1) {
            mvwprintw(input_win, 1, 1, "Enter your message: ");
            wrefresh(input_win);

            if (fgets(msg_buffer, msg_b_size, stdin)) {
                int bytes_sent = send(sock_fd, msg_buffer, strlen(msg_buffer), 0);
                wprintw(input_win, "Sent %d bytes (%s) to server\n", bytes_sent, msg_buffer);
                wrefresh(input_win);

                memset(msg_buffer, 0, sizeof msg_buffer);
            }
        }
    }

    while (1) {
        int to_recv = poll(watch_list, 1, 1000);
        if (to_recv) {
            if (watch_list[0].revents & POLLHUP) {
                mvprintw(0, 0, "\nThe server closed connection\n");
                refresh();
                break;
            }
            if (watch_list[0].revents & POLLIN) {
                int bytes_received = recv(sock_fd, recv_msg_buffer, recv_b_size, 0);
                if (bytes_received <= 0) {
                    mvprintw(0, 0, "\nThe server closed connection\n");
                    refresh();
                    break;
                }
                mvprintw(0, 0, "Message from server: %s\n", recv_msg_buffer);
                refresh();
                memset(recv_msg_buffer, 0, sizeof recv_msg_buffer);
            }
        }
    }
    close(sock_fd);
    endwin();
    return 0;
}
