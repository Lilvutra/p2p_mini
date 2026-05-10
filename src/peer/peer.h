#ifndef PEER_H
#define PEER_H

#include <pthread.h>
#include <arpa/inet.h>
#include "../config.h"

typedef struct {
    int sock;
    char ip[INET_ADDRSTRLEN];
    int port;
} Peer;

extern Peer peers[MAX_PEERS];
extern int peer_count;
extern int my_port;
extern pthread_mutex_t peers_mutex;

void add_peer(int sock, char *ip, int port);
void remove_peer(int sock);
void update_peer_port(int sock, int new_port);

#endif