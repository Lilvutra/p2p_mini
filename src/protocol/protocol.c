#include "protocol.h"
#include "../peer/peer.h"
#include "../network/network.h"

#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <pthread.h>
#include <stdlib.h>


// Broadcast message to all peers
void broadcast(char *msg, int sender_sock) {
    pthread_mutex_lock(&peers_mutex);

    for (int i = 0; i < peer_count; i++) {
        int sock = peers[i].sock;

        if (sock != sender_sock) { // except sender
            send(sock, msg, strlen(msg), 0); // send raw message
        }
    }

    pthread_mutex_unlock(&peers_mutex);
}

// Handle known hosts
void handle_known_hosts(int requester_sock){
    char response[MAX_PEERS * (INET_ADDRSTRLEN+6)];
    strcpy(response, "HOSTS:");

    pthread_mutex_lock(&peers_mutex);

    // loop through each known_hosts
    for (int i = 0; i< peer_count; i++){
        // except requester itself
        // if requester is already in known_hosts, it means requester is already connected, so we should not send requester its own ip:port back, otherwise requester will try to connect to itself and cause error. So we should skip requester when building the response.
        // should we return sth if requester is already in known_hosts? I think we can just return empty list, which means requester is already connected to all known hosts, so there is no new host to connect. So if requester is already in known_hosts, we can just skip requester and return the rest of the known_hosts.
        if (peers[i].sock != requester_sock) {
            // build ip:port then append to response
            char entry[INET_ADDRSTRLEN+6];
            snprintf(entry, sizeof(entry), "%s:%d,",peers[i].ip, peers[i].port );
            strcat(response, entry);
        }
     

    }
    pthread_mutex_unlock(&peers_mutex);
    strcat(response, "\n");
    send(requester_sock, response, strlen(response), 0);
}
