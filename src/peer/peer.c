#include "peer.h"
#include <stdio.h>
#include <string.h>
#include <pthread.h>
Peer peers[MAX_PEERS];
int peer_count = 0;
int my_port;
pthread_mutex_t peers_mutex;

// Add peer safely
void add_peer(int sock, char *ip, int port) {
    pthread_mutex_lock(&peers_mutex);

    if (peer_count < MAX_PEERS) {
        peers[peer_count].sock = sock;
        peers[peer_count].port = port;
        strcpy(peers[peer_count].ip, ip);
        peer_count++;
        printf("New peer added. Total: %d\n", peer_count);
    }

    pthread_mutex_unlock(&peers_mutex);
}


void remove_peer(int sock) {
    pthread_mutex_lock(&peers_mutex);

    for (int i = 0; i < peer_count; i++) {
        if (peers[i].sock == sock) {
            peers[i] = peers[peer_count-1];
            peer_count--;
            break;
        }
    }

    pthread_mutex_unlock(&peers_mutex);
}

void update_peer_port (int sock, int new_port) {
    pthread_mutex_lock(&peers_mutex);
    for (int i = 0; i < peer_count; i++) {
        if (peers[i].sock == sock ){
            peers[i].port = new_port;
            break;
        }
    }
    pthread_mutex_unlock(&peers_mutex);
}