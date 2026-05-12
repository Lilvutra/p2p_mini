#include "peer.h"
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
Peer peers[MAX_PEERS];
int peer_count = 0;
int my_port;
pthread_mutex_t peers_mutex;
/*
// Add peer safely
void add_peer(int sock, char *ip, int port) {
    pthread_mutex_lock(&peers_mutex);

    if (peer_count < MAX_PEERS) {
        // we should check if this peer is already in known_hosts before adding, because when two peers connect to each other at the same time, they will both add each other to known_hosts, which will cause duplicate entry in known_hosts and cause problem when we try to connect to known_hosts later. So we should check if this peer is already in known_hosts before adding, if it is already in known_hosts, we should not add it again.
        int already_in_known_hosts = 0;
        for (int i = 0; i < peer_count; i++) {
            if (strcmp(peers[i].ip, ip) == 0 && peers[i].port == port) {
                already_in_known_hosts = 1;
                break;
            }
        }
        if (already_in_known_hosts) {
            printf("Peer %s:%d is already in known_hosts, skipping add_peer\n", ip, port);
            pthread_mutex_unlock(&peers_mutex);
            return;
        }
        peers[peer_count].sock = sock;
        peers[peer_count].port = port;
        strcpy(peers[peer_count].ip, ip);
        peer_count++;
        //there are two messages New peer added when connecting two peers? because each peer will connect to the other, so each peer will print "New peer added" when it receives the connection from the other peer. So if you see two "New peer added" messages, it means both peers have successfully connected to each other.
        printf("New peer added. Total: %d\n", peer_count);
    }

    pthread_mutex_unlock(&peers_mutex);
}
*/

void add_peer(int sock, char *ip, int port) {
    pthread_mutex_lock(&peers_mutex);
     // currently we allow duplicate peers (same ip:port) in known_hosts, because it is possible that two different peers have the same ip:port (e.g. behind NAT), 
     // so we should not rely on ip:port to determine if a peer is already in known_hosts, instead we should rely on socket to determine if a peer is already in known_hosts, 
     // because each connection will have a unique socket, so if a socket is already in known_hosts, it means this peer is already in known_hosts, we should not add it again. 
     // So we should check if this socket is already in known_hosts before adding, if it is already in known_hosts, we should not add it again.
     // however, the accept func will take ephemeral port for incoming connection, so the port in known_hosts will be the ephemeral port, 
     // which is not useful for other peers to connect to this peer, so we should update the port in known_hosts after receiving the "Hello:port" message from this peer, 
     // how can we wait for handshake then extract real indentity 
     // 1. Check if peer already exists (by ip + port OR socket)
    for (int i = 0; i < peer_count; i++) {

        // Same logical node (most important check)
        if (strcmp(peers[i].ip, ip) == 0 &&
            peers[i].port == port) {

            printf("[INFO] Peer %s:%d already exists, closing duplicate socket\n", ip, port);

            // avoid leaking duplicate sockets
            close(sock);

            pthread_mutex_unlock(&peers_mutex);
            return;
        }

        // Same socket (already added via another path)
        if (peers[i].sock == sock) {
            pthread_mutex_unlock(&peers_mutex);
            return;
        }
    }

    // 2. Add new peer safely
    if (peer_count >= MAX_PEERS) {
        printf("[WARN] Peer limit reached\n");
        close(sock);
        pthread_mutex_unlock(&peers_mutex);
        return;
    }

    peers[peer_count].sock = sock;
    peers[peer_count].port = port;
    strcpy(peers[peer_count].ip, ip);

    peer_count++;

    printf("[%d][INFO] New peer added %s:%d (total=%d)\n",
           my_port, ip, port, peer_count);

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

// Check already connected - avoid duplicate connection
int already_connected(char *ip, int port) {
    pthread_mutex_lock(&peers_mutex);
    // loop through peers
    for (int i=0; i< peer_count; i++) {
        // compare each peer ip 
        if (strcmp(peers[i].ip, ip) ==0 && peers[i].port == port) {
            pthread_mutex_unlock(&peers_mutex);
            return 1; // found
        }
    }
    pthread_mutex_unlock(&peers_mutex);
    return 0; //not found
}