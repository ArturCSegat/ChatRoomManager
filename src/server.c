#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <uchar.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include <signal.h>
#include "../headers/din_arr.h"
#include "../headers/chat_room.h"
#include "../headers/network_utils.h"

int running = 1;

typedef struct {
    struct chatroom ** rooms;
    int len;
    int cap;
}rooms_arr;

void stop(int sig) {
    running = 0;
}

void add_room(rooms_arr * rooms, struct chatroom * room, int listener) {
    if (rooms->len == rooms->cap) {
        rooms->cap *= 1.5;
        rooms->rooms = realloc(rooms->rooms, sizeof(struct chatroom *) * rooms->cap);
    }
    rooms->rooms[rooms->len] = room;

    add_con(rooms->rooms[rooms->len], listener);
    append_str(rooms->rooms[rooms->len]->names, "server", 6);

    rooms->len += 1;
}

void list_rooms(rooms_arr * rooms, int to_fd) {
    char * m2 = "type: - :join room name - to join a room or - :new room name - to create a new one\n";
    send(to_fd, m2, strlen(m2), 0);
    for (int i = 1; i < rooms->len; i++){
        char m[60];
        sprintf(m, "room: %s\n", rooms->rooms[i]->name); 
        send(to_fd, m, strlen(m), 0);
    }
}

int main(void) {
    signal(SIGINT, stop);

    int listen_socket = get_listening_sock_or_die();

    printf("listening on %d :...\n", listen_socket);
        
    rooms_arr rooms;
    rooms.len = 0;
    rooms.cap = 3;
    rooms.rooms = malloc(sizeof(struct chatroom *) * rooms.cap);

    add_room(&rooms, chatroom_builder(0, "Room Picker"), listen_socket);

    // main accept loop
    while (running) {
        for (int i = 0; i < rooms.len; i++) {
            printf("%s\n", rooms.rooms[i]->name);
            din_arr * senders = new_din_arr(3);
            din_arr * new_connections = new_din_arr(3);
            if (recv_conns(rooms.rooms[i], listen_socket, senders, new_connections) != 0) {
                printf("error happend when handling new connections\n");
                printf("errno (may not be related: %d", errno);
                perror("may not be related");
            }

            if (i == 0) {
                for (int j = 0; j < new_connections->len; j++) {
                    printf("opa\n");
                    list_rooms(&rooms,  new_connections->arr[j]);
                } 
            }
            
            if (senders->len > 0) {
                for (int j = 0; j < senders->len; j++) {
                    int senders_idx = index_of_fd(rooms.rooms[i], senders->arr[j]);
                    char msg_buff[200];
                    int bytes = recv(senders->arr[j], msg_buff, sizeof msg_buff, 0);
                    msg_buff[bytes] = 0;

                    if (bytes <= 0) {
                        close(senders->arr[j]);
                        remove_con(rooms.rooms[i], senders->arr[j]);
                        char server_msg[100];
                        snprintf(server_msg, sizeof server_msg, "%s has left the channel from socket %d\n", rooms.rooms[i]->names->arr[senders_idx], senders->arr[j]);
                        pop_str(rooms.rooms[i]->names, senders_idx);
                        spread_msg(rooms.rooms[i], server_msg, "server", listen_socket);
                        continue;
                    }
                    
                    if (!strncmp(msg_buff, ":new ", 5)) {
                        char chan_name[50];   
                        memset(chan_name, 0, sizeof chan_name);
                        for( int k = 5; k < strnlen(msg_buff, 50); k++) {
                            chan_name[k-5] = msg_buff[k];
                        }
                        add_room(&rooms, chatroom_builder(rooms.len, chan_name), listen_socket);
                        add_con(rooms.rooms[rooms.len -1], senders->arr[j]);
                        append_str(rooms.rooms[rooms.len -1]->names, rooms.rooms[i]->names->arr[senders_idx], strlen(rooms.rooms[i]->names->arr[senders_idx]));
                        remove_con(rooms.rooms[i], senders->arr[j]);
                        pop_str(rooms.rooms[i]->names, senders_idx);
                        pop(senders, i);
                        
                        memset(chan_name, 0, sizeof chan_name);
                        continue;
                    }

                    if (!strncmp(msg_buff, ":join ", 6)) {
                        char chan_name[50];   
                        memset(chan_name, 0, sizeof chan_name);
                        for( int k = 6; k < strnlen(msg_buff, 50); k++) {
                            chan_name[k-6] = msg_buff[k];
                        }
                        int room_idx = -1;
                        for (int k = 0; k < rooms.len; k++) {
                            if (!strcmp(rooms.rooms[k]->name, chan_name)) {
                                room_idx = k;
                                break;
                            }
                        }
                        if (room_idx == -1) {
                            char * m =  "server says: invalid channel name\n";
                            send(senders->arr[j], m, strlen(m), 0);
                            continue;
                        }

                        add_con(rooms.rooms[room_idx], senders->arr[j]);
                        append_str(rooms.rooms[room_idx]->names, rooms.rooms[i]->names->arr[senders_idx], strlen(rooms.rooms[i]->names->arr[senders_idx]));
                        remove_con(rooms.rooms[i], senders->arr[j]);
                        pop_str(rooms.rooms[i]->names, senders_idx);
                        pop(senders, i);
                        
                        memset(chan_name, 0, sizeof chan_name);
                        continue;
                    }

                    if (!strncmp(msg_buff, ":leave", 6)) {
                        remove_con(rooms.rooms[i], senders->arr[j]);
                        pop_str(rooms.rooms[i]->names, senders_idx);
                        pop(senders, i);
                        list_rooms(&rooms, senders->arr[j]);
                        continue;
                    }

                    printf("voce esta no canal %s gamer\n", rooms.rooms[i]->name);
                    spread_msg(rooms.rooms[i], msg_buff, rooms.rooms[i]->names->arr[senders_idx], listen_socket);

                    memset(msg_buff, 0, sizeof(msg_buff));
                }
            }
            free_din_arr(senders);
            free_din_arr(new_connections);
        }
    }
    
    for (int i = 0; i < rooms.len; i++) {
        printf("lolz\n");
        free_chat_room(rooms.rooms[i]);
    }
    free(rooms.rooms);
}
