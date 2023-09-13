#include "din_arr.h"

#ifndef CHAT_ROOM_H
#define CHAT_ROOM_H

struct chatroom {
    int id;
    char name[50];
    struct pollfd * fds;
    str_din_arr * names;
    int fds_len;
    int fds_cap;
};

struct chatroom * chatroom_builder(int id, char name[50]);

void print_chatroom(struct chatroom cr); 

void free_chat_room(struct chatroom * cr);

void add_con(struct chatroom * cr, int fd);

void remove_con(struct chatroom * cr, int fd);

void print_listners(struct chatroom cr);

// fill the sender_buf with all the sockets that want to send and the msgs_buff with the messages the senders want to send
// will also accept any new connections to the listenr
int recv_conns(struct chatroom * cr, int listener, din_arr * senders, din_arr * new_conns);

void spread_msg(struct chatroom * cr, const char * msg, const char * sayer_name, int sender_fd);

int index_of_fd(struct chatroom * cr, int fd);

#endif
