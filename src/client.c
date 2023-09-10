#include <curses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <poll.h>
#include <stdio_ext.h>
#include <signal.h>
#include <ncurses.h>
#include "../headers/network_utils.h"


# define MAX_NAME 20
# define MAX_MSG 200

int running = 1;

int main(int argc, char *argv[]) {
    

    char *server_ip = argv[1];

    if (argc != 2) {
        printf("To specify a server's IP Address, please run: %s <server_ip>. connecting to 127.0.0.1\n", argv[0]);
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

    // ncurses initialization
    initscr(); 
    curs_set(1); // Show cursor

    WINDOW * output_window = newwin(LINES - 2, COLS, 0, 0);
    scrollok(output_window, TRUE);
    wrefresh(output_window);

    WINDOW * input_window = newwin(1, COLS, LINES - 1, 0);
    keypad(input_window, TRUE);
    wprintw(input_window, "enter your message: ");
    wrefresh(input_window);
    
    if (!fork()) {
        char msg_buffer[MAX_MSG];
        int msg_b_size = sizeof msg_buffer;

        while(running) {
            wgetnstr(input_window, msg_buffer, msg_b_size);

            if (!strcmp(msg_buffer, ":quit")) {
                running = 0;
                break;
            }
            
            wclrtoeol(input_window);
            wprintw(input_window, "enter your message: ");
            wrefresh(input_window);


            int bytes_sent = send(sock_fd, msg_buffer, strlen(msg_buffer), 0);
            
            memset(msg_buffer, 0, strlen(msg_buffer));
        }
        printf("end of child\n");
        close(sock_fd);
        exit(0);
    }

    char recv_msg_buffer[MAX_MSG];
    int recv_b_size = sizeof recv_msg_buffer;

    int row = 1;
    while(running) {
        int bytes_received = recv(sock_fd, recv_msg_buffer, recv_b_size, 0);

        if (bytes_received <= 0) {
            printf("\nThe server closed connection\n");
            running = 0;
            break;
        }
        recv_msg_buffer[bytes_received] = 0;

        mvwprintw(output_window, row, 1, "%s", recv_msg_buffer);
        row += 1;
        wrefresh(output_window);

        wmove(input_window, 1, 1);
        wrefresh(input_window);

        memset(recv_msg_buffer, 0, strlen(recv_msg_buffer));
    }
    delwin(input_window);
    delwin(output_window);
    endwin();
    printf("end of parent\n");
    close(sock_fd);
    return 0;
}
