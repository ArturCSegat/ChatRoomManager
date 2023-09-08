#ifndef NETWORK_UTILS_H
#define NETWORK_UTILS_H

# define PORT "6969"

void *get_in_addr(struct sockaddr *sa);

struct addrinfo * get_sock_info(const char * ip);

int get_server_sock_or_die(const char * server_ip); 
int get_listening_sock_or_die(); 

#endif
