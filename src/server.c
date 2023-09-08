#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <netdb.h>
#include <poll.h>
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
                char msg_buff[200];
                int bytes = recv(senders->arr[i], msg_buff, sizeof msg_buff, 0);
                
                if (bytes <= 0) {
                    close(senders->arr[i]);
                    remove_con(test_room, senders->arr[i]);
                    printf("closed connection from %d\n", senders->arr[i]);
                        sleep(2);
                    continue;
                }
                
                spread_msg(test_room, msg_buff, listen_socket, senders->arr[i]);

                printf("spreading message %d bytes long from %d (%s)\n", bytes, senders->arr[i], msg_buff);
                memset(msg_buff, 0, sizeof(msg_buff));
            }
        }
        free_din_arr(senders);
    }
    
    free_chat_room(test_room);
}
