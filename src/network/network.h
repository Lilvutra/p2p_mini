#ifndef NETWORK_H
#define NETWORK_H

void connect_to_peer(char *ip, int port);
void parse_n_connect(char *payload);
void* handle_peer(void *arg);
void* server_thread(void *arg);
void* input_thread(void *arg);

#endif