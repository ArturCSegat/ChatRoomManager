#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <poll.h>
#include <ncurses.h>
#include <pthread.h>
#include "../headers/network_utils.h"

WINDOW *output_window;
WINDOW *input_window;
char msg_buffer[200];
int msg_b_size;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void *receive_messages(void *arg) {
    int sock_fd = *((int *)arg);

    int first_message_received = 0;

    while (1) {
        char recv_msg_buffer[200];
        int recv_b_size = sizeof recv_msg_buffer;

        int bytes_received = recv(sock_fd, recv_msg_buffer, recv_b_size - 1, 0);
        if (bytes_received <= 0) {
            break; // Server closed connection
        }
        recv_msg_buffer[bytes_received] = '\0';

        pthread_mutex_lock(&mutex);
        wprintw(output_window, "\nThem: %s\n", recv_msg_buffer);
        wrefresh(output_window);
        pthread_mutex_unlock(&mutex);

        memset(recv_msg_buffer, 0, sizeof recv_msg_buffer);

        if (!first_message_received) {
            pthread_mutex_lock(&mutex);
            wrefresh(output_window);
            pthread_mutex_unlock(&mutex);
            first_message_received = 1;
        }
    }

    return NULL;
}

int main(int argc, char *argv[]) {
    char *server_ip = argv[1];

    if (argc != 2) {
        printf("To specify a server's IP Address, please run: %s <server_ip>. Attempting to connect anyways...\n", argv[0]);
        server_ip = NULL;
    }

    // Sleep for 3 seconds
    sleep(3);

    int sock_fd = get_server_sock_or_die(server_ip);
    msg_b_size = sizeof msg_buffer;

    struct pollfd watch_list[1];
    watch_list[0].fd = sock_fd;
    watch_list[0].events = POLLIN;

    initscr();
    noecho();
    curs_set(1); // Show cursor

    output_window = newwin(LINES - 2, COLS, 0, 0);
    scrollok(output_window, TRUE);
    wrefresh(output_window);

    input_window = newwin(1, COLS, LINES - 1, 0);
    keypad(input_window, TRUE);
    wprintw(input_window, "enter your message: ");
    wrefresh(input_window);

    pthread_t recv_thread;
    if (pthread_create(&recv_thread, NULL, receive_messages, &sock_fd) != 0) {
        perror("pthread_create");
        return 1;
    }

    int ch;
    int input_pos = 0;

    while (1) {
        ch = wgetch(input_window);
        if (ch != ERR) {
            if (ch == '\n') {
                int bytes_sent = send(sock_fd, msg_buffer, strlen(msg_buffer), 0);
                pthread_mutex_lock(&mutex);
                wprintw(output_window, "\nYou: %s\n", msg_buffer);
                wrefresh(output_window);
                pthread_mutex_unlock(&mutex);
                memset(msg_buffer, 0, sizeof msg_buffer);
                input_pos = 0;
                wmove(input_window, 0, 19);
                wclrtoeol(input_window);
                wrefresh(input_window);
            } else if (ch == KEY_BACKSPACE || ch == 127) {
                if (input_pos > 0) {
                    input_pos--;
                    msg_buffer[input_pos] = '\0';
                    wmove(input_window, 0, 19 + input_pos);
                    wclrtoeol(input_window);
                    wrefresh(input_window);
                }
            } else {
                if (input_pos < msg_b_size - 1) {
                    msg_buffer[input_pos] = ch;
                    input_pos++;
                    mvwaddch(input_window, 0, 19 + input_pos, ch);
                    wrefresh(input_window);
                }
            }
        }
    }

    endwin();
    close(sock_fd);

    pthread_join(recv_thread, NULL);

    return 0;
}
