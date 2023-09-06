#include <poll.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <stdio.h>
#include <unistd.h>

struct chatroom {
    int id;
    char name[50];
    struct pollfd * fds;
    int fds_len;
    int fds_cap;
};

struct chatroom * chatroom_builder(int id, char name[50]) {
    struct chatroom * cr = (struct chatroom * )malloc(sizeof(struct chatroom));
    
    cr->id = id;
    for (int i = 0; i < strlen(name); i++) {
        cr->name[i] = name[i];
    }
    
    // arbitrary starting number of fd's capacity (3)
    cr->fds = (struct pollfd * )malloc(sizeof(struct pollfd) * 3);
    cr->fds_len = 0;
    cr->fds_cap = 3;

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
}

void free_chat_room(struct chatroom * cr) {
    free(cr->fds);
    free(cr);
}

void add_con(struct chatroom * cr, int fd) {
    if (cr->fds_len + 1 == cr->fds_cap) {
        cr->fds_cap *= 1.5;
        cr->fds = (struct pollfd * )realloc(cr->fds, cr->fds_cap);
    }
    cr->fds[cr->fds_len].fd = fd; 
    cr->fds[cr->fds_len].events = POLLIN; 
    cr->fds_len += 1;
    printf("before: \n");
    print_chatroom(*cr);
    printf("after: \n");
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

// fill the sender_buf with all the sockets that want to send and the msgs_buff with the messages the senders want to send
// will also accept any new connections to the listenr
int receive_conns_spread_msgs(struct chatroom * cr, int listener) {
    printf("function start\n");
    int evented_count = poll(cr->fds, cr->fds_len, -1);
    printf("evented: %d\n", evented_count);
    if (evented_count == -1) {
        return 1;     
    }
    
    for (int i = 0; i < cr->fds_len; i++) {
        printf("cr->fds[i]: %d\n", cr->fds[i].fd);
        if (cr->fds[i].events & POLLIN) { // if has event
            printf("has event\n");
            if (cr->fds[i].fd == listener) {
                // accept any new connections
                printf("is listener\n");
                struct sockaddr new_con;
                socklen_t new_c_size = sizeof new_con;

                int new_fd = accept(listener, &new_con, &new_c_size);

                if (new_fd == - 1) {
                    perror("failed to accept");
                } else {
                    add_con(cr, new_fd);
                    printf("new connection from %d", new_fd);
                }
                continue;
            }
            // any connections 
            printf("isnt listener\n");
            char msg_buf[200];
            int received_bytes = recv(cr->fds[i].fd, msg_buf, sizeof msg_buf, 0);            

            if (received_bytes <= 0) {
                if (received_bytes == 0) {
                    printf("socket %d hung up\n", cr->fds[i].fd);
                } else {
                    perror("recv");
                }
                close(cr->fds[i].fd);
                remove_con(cr, cr->fds[i].fd);
                continue;
            } 
            
            for (int j = 0; j < cr->fds_len; j++) {
                printf("cr->fds[j]: %d\n", cr->fds[j].fd);
                if (cr->fds[j].fd == cr->fds[i].fd || cr->fds[j].fd == listener) {printf("invalid\n");continue;};
                if (send(cr->fds[j].fd, msg_buf, received_bytes, 0) == -1) {
                    perror("send");
                }
            }
            
        }
    }
    printf("function end\n");
    return 0;
}

