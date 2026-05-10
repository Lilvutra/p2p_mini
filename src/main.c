#include "config.h"
#include "peer/peer.h"
#include "network/network.h"
#include "protocol/protocol.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <string.h>

// Peer peers[MAX_PEERS];
// int peer_count = 0;
// int my_port;
// pthread_mutex_t peers_mutex;


// MAIN
int main(int argc, char *argv[]) {
    // setbuf(stdout, NULL);
    if (argc < 2) {
        printf("Usage: %s <port> [peer_ip peer_port]\n", argv[0]);
        return 1;
    }

    int port = atoi(argv[1]);
    my_port = port;

    pthread_mutex_init(&peers_mutex, NULL);

    // Create server socket
    int server_fd = socket(AF_INET, SOCK_STREAM, 0); // (domain, type, protocol)

    // config socket
    // enable port reused
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)); //(socket, level, option, value_ptr, pass)

    struct sockaddr_in server_addr;
    // clear memory (fill block with 0 to avoid garbage memory)
    memset(&server_addr, 0, sizeof(server_addr)); // (pointer, value, size)
    server_addr.sin_family = AF_INET; // address type
    server_addr.sin_port = htons(port); // port number
    server_addr.sin_addr.s_addr = INADDR_ANY; // IP address

    // attach socket to port
    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) <0) {
        perror("Bind failed");
        exit(1);
    };
    listen(server_fd, 10);

    printf("Node listening on port %d\n", port);

    // Start server thread
    pthread_t server_tid;
    pthread_create(&server_tid, NULL, server_thread, &server_fd);

    // Start input thread
    pthread_t input_tid;
    pthread_create(&input_tid, NULL, input_thread, NULL);

    // Optional: connect to peer
    if (argc == 4) {
        connect_to_peer(argv[2], atoi(argv[3]));
    }

    pthread_join(server_tid, NULL);
    pthread_join(input_tid, NULL);

    return 0;
}