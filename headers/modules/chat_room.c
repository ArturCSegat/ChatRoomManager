#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <poll.h>
#include "../din_arr.h"
#include "../chat_room.h"

struct chatroom * chatroom_builder(int id, char name[50]) {
    struct chatroom * cr = (struct chatroom * )malloc(sizeof(struct chatroom));
    
    cr->id = id;
    memset(cr->name, 0, sizeof(cr->name));
    for (int i = 0; i < strlen(name); i++) {
        cr->name[i] = name[i];
    }
    
    // arbitrary starting number of fd's capacity (3)
    cr->fds = (struct pollfd * )malloc(sizeof(struct pollfd) * 3);
    cr->fds_len = 0;
    cr->fds_cap = 3;
    cr->names = new_str_din_arr(3);

    return cr;
}

void print_chatroom(struct chatroom cr) {
    printf("id: %d\n", cr.id);
    printf("name: %s\n", cr.name);
    printf("poll group: [");
    for (int i = 0; i < cr.fds_len; i++) {
        printf("{s:%d e:%d r:%d} ", cr.fds[i].fd, cr.fds[i].events, cr.fds[i].revents);
    }
    printf("]\n");
    printf("len: %d\n", cr.fds_len);
    printf("cap: %d\n", cr.fds_cap);
    printf("names: ");
    print_strdinarr(*cr.names);
}

void free_chat_room(struct chatroom * cr) {
    free_str_din_arr(cr->names);
    free(cr->fds);
    free(cr);
}

void add_con(struct chatroom * cr, int fd) {
    if (cr->fds_len + 1 == cr->fds_cap) {
        cr->fds_cap *= 1.5;
        cr->fds = (struct pollfd * )realloc(cr->fds, cr->fds_cap * sizeof(struct pollfd));
    }
    cr->fds[cr->fds_len].fd = fd; 
    cr->fds[cr->fds_len].events = POLLIN; 
    cr->fds_len += 1;
}

void remove_con(struct chatroom * cr, int fd) {
    for (int i = 0; i < cr->fds_len; i++) {
        if (cr->fds[i].fd == fd) {
            cr->fds[i] = cr->fds[cr->fds_len - 1];
            cr->fds_len -= 1;
            break;
        }
    }
}

void print_listners(struct chatroom cr){
    printf("[");
    for (int i = 0; i < cr.fds_len; i++) {
        printf("%d, ", cr.fds[i].fd);
    }
    printf("]\n");
}

// fill the sender_buf with all the sockets that want to send and the msgs_buff with the messages the senders want to send
// will also accept any new connections to the listenr
int recv_conns(struct chatroom * cr, int listener, din_arr * senders, din_arr * new_conns) {
    if (senders->len > 0) {
        return  -1;
    }

    int evented_count = poll(cr->fds, cr->fds_len, 1500); // 1500 ms
    if (evented_count == -1) {
        return -1;     
    }

    for (int i = 0; i < cr->fds_len; i++) {

        if( cr->fds[i].revents & POLLIN) {  // if can read from this socket
            if (cr->fds[i].fd == listener) {
                // accept any new connections
                struct sockaddr new_con;
                socklen_t new_c_size = sizeof new_con;

                int new_fd = accept(listener, &new_con, &new_c_size);

                if (new_fd == - 1) {
                    perror("failed to accept");
                } else {
                    add_con(cr, new_fd);
                    append(new_conns, new_fd);

                    char name_buff[20];
                    int name_bytes = recv(new_fd, name_buff, sizeof name_buff, 0);
                    
                    append_str(cr->names, name_buff, name_bytes);

                    memset(name_buff, 0, name_bytes);
                    
                    char server_msg[100];
                    snprintf(server_msg, sizeof server_msg, "%s has joined the channel from socket %d\n", cr->names->arr[cr->names->len - 1], new_fd);
                    spread_msg(cr, server_msg, "server", listener);

                    
                }
                continue;
            }
            // any connections 
            append(senders, cr->fds[i].fd);
        }
    }
    return 0;
}

void spread_msg(struct chatroom * cr, const char * msg, const char * sayer_name, int sender_fd) {

    if (!strcmp(cr->name, "Room Picker")){return;} // dont want to send messages in the lobby channel
    
    char full_message[sizeof(msg) + sizeof(sayer_name) + 100];
    memset(full_message, 0, sizeof full_message);
    snprintf(full_message, sizeof full_message, "%s says: %s\n", sayer_name, msg);

    printf("spreading message: %s on %s\n", full_message, cr->name);
    for (int j = 1; j < cr->fds_len; j++) {
        if (cr->fds[j].fd == sender_fd) {
            continue;
        }

        if (send(cr->fds[j].fd, full_message, strlen(full_message), 0) == - 1) {
            printf("error on sending to %d\n", cr->fds[j].fd);
           perror("send");
        }
    }
}


int index_of_fd(struct chatroom * cr, int fd) {
    int idx = -1;
    for (int i = 0; i < cr->fds_len; i++) {
        if (cr->fds[i].fd == fd) {
            idx = i;
            break;
        }
    }    
    
    return idx;
}
