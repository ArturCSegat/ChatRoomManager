#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/mman.h>
#include <poll.h>
#include <stdio_ext.h>
#include <ncurses.h>
#include "../headers/network_utils.h"
#include <threads.h>

# define MAX_NAME 20
# define MAX_MSG 200

WINDOW * output_window;
WINDOW * input_window;

atomic_int running = 1;
atomic_int row = 1;

int sock_fd;

int handle_stdin_input(void * thread_data) {
    while (running) {
        char msg_buffer[MAX_MSG];
        int msg_b_size = sizeof msg_buffer;

        wgetnstr(input_window, msg_buffer, msg_b_size);

        if (!strcmp(msg_buffer, ":quit")) {
            running = 0;
            int bytes_sent = send(sock_fd, ":leave", strlen(":leave"), 0); // makes sure the servers removes the user from the channel
            break;
        }
        if (!strncmp(msg_buffer, ":new ", 5) || !strncmp(msg_buffer, ":join ", 6) || !strncmp(msg_buffer, ":leave", 6)) {
            row = 1;
            wclear(output_window);
            wrefresh(output_window);
        }

        wclrtoeol(input_window);
        wprintw(input_window, "enter your message: ");
        wrefresh(input_window);


        int bytes_sent = send(sock_fd, msg_buffer, strlen(msg_buffer), 0);

        memset(msg_buffer, 0, strlen(msg_buffer));
    }
    return 0;
}

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
    
    sock_fd = get_server_sock_or_die(server_ip);
    send(sock_fd, name, strlen(name), 0);
    memset(name, 0, MAX_NAME);
    printf("\n");

    // ncurses initialization
    initscr(); 
    curs_set(1); // Show cursor

    output_window = newwin(LINES - 2, COLS, 0, 0);
    scrollok(output_window, TRUE);
    wrefresh(output_window);

    input_window = newwin(1, COLS, LINES - 1, 0);
    keypad(input_window, TRUE);
    wprintw(input_window, "enter your message: ");
    wrefresh(input_window);

    thrd_t input_thread;
    if (thrd_create(&input_thread, handle_stdin_input, NULL) != thrd_success) {
        printf("failed to create input thread\n");
        running = 0;
    }

    char recv_msg_buffer[MAX_MSG];
    int recv_b_size = sizeof recv_msg_buffer;

    while(running) {
        int bytes_received = recv(sock_fd, recv_msg_buffer, recv_b_size, 0);

        if (bytes_received <= 0) {
            running = 0;
            break;
        }
        recv_msg_buffer[bytes_received] = 0;

        mvwprintw(output_window, row, 1, "%s", recv_msg_buffer);
        row += 1;
        wrefresh(output_window);

        wmove(input_window, 1, 1);
        wrefresh(input_window);
        
        memset(recv_msg_buffer, 0, sizeof recv_msg_buffer);
    }
    thrd_join(input_thread, NULL);

    delwin(input_window);
    delwin(output_window);
    endwin();
    printf("you quited or the server closed =(\n");
    close(sock_fd);
    return 0;
}
