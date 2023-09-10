#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include "../headers/din_arr.h"
#include "../headers/chat_room.h"
#include "../headers/network_utils.h"

int main(void) {
    int listen_socket = get_listening_sock_or_die();

    printf("listening on %d :...\n", listen_socket);
    
    struct chatroom * test_room = chatroom_builder(0, "sala teste");
    add_con(test_room, listen_socket);
    append_str(test_room->names, "server", 6);

    // main accept loop
    while (1) {
        din_arr * senders = new_din_arr(3);
        if (recv_conns(test_room, listen_socket, senders) != 0) {
            printf("error happend when handling new connections\n");
            printf("errno (may not be related: %d", errno);
            perror("may not be related");
        }
        if (senders->len > 0) {
            for (int i = 0; i < senders->len; i++) {
                int senders_idx = index_of_fd(test_room, senders->arr[i]);
                char msg_buff[200];
                int bytes = recv(senders->arr[i], msg_buff, sizeof msg_buff, 0);
                
                if (bytes <= 0) {
                    close(senders->arr[i]);
                    remove_con(test_room, senders->arr[i]);
                    char server_msg[100];
                    snprintf(server_msg, sizeof server_msg, "%s has left the channel from socket %d\n", test_room->names->arr[senders_idx], senders->arr[i]);
                    pop_str(test_room->names, senders_idx);
                    spread_msg(test_room, server_msg, "server", listen_socket);
                    continue;
                }
                
                spread_msg(test_room, msg_buff, test_room->names->arr[senders_idx], senders->arr[i]);

                memset(msg_buff, 0, sizeof(msg_buff));
            }
        }
        free_din_arr(senders);
    }
    
    free_chat_room(test_room);
}
