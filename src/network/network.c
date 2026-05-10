#include "network.h"
#include "../peer/peer.h"
#include "../protocol/protocol.h"
#include "../config.h"
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <stdio.h>


// Handle incoming messages
void* handle_peer(void *arg) { // each connection gets 1 thread
    int sock = *(int*)arg; // extract socket
    free(arg);

    char buffer[BUFFER_SIZE];
    char temp_buffer[BUFFER_SIZE]; // temp storage to store message chunk
    int buffer_len = 0; // number of bytes currently in buffer

    while (1) {
        // receive temp_buffer
        int temp_bytes = recv(sock, temp_buffer, BUFFER_SIZE - 1, 0); // get incoming message
        if (temp_bytes <= 0) {
                printf("Peer disconnected\n");
                close(sock);
                remove_peer(sock);
                break;
        }
        if (buffer_len + temp_bytes >= BUFFER_SIZE) {
            printf("Buffer overflow, resetting...");
            buffer_len = 0;
            continue;
        }

        // append temp_buffer to buffer
        memcpy(buffer + buffer_len, temp_buffer, temp_bytes);  // (dest, src, size)
        buffer_len += temp_bytes;
        buffer[buffer_len] = '\0';  // null terminator

        // while buffer contains '\n': extract 1 message, process it , remove it from buffer
        while (strchr(buffer, '\n')){
            char *newline = strchr(buffer, '\n'); // address of '\n' inside buffer
            int msg_len = newline - buffer;  // (address of newline) - (start of buffer)
            
            // Extract message
            char message[BUFFER_SIZE];
            memcpy(message, buffer, msg_len);  // (dest, src, size) - copy 'size' bytes from "src" to "dest"
            message[msg_len] = '\0';

            // Print
            printf("Received: %s\n", message);
            // forward message to others
            if (strncmp(message, "/known_hosts", 12) == 0) {
                handle_known_hosts(sock);
                //printf("known_hosts requested \n");
            } else if(strncmp(message, "Hello:", 6) == 0){
                // extract port number from "Hello:5001"
                int their_port = atoi(message +6);
                //update this peer's port in peers[]
                update_peer_port(sock, their_port);
            }
            else {
                broadcast(message, sock);
            }
            // Remove processed message from buffer
            int remaining = buffer_len - (msg_len + 1);
            memmove(buffer, newline + 1, remaining);

            buffer_len = remaining;
            buffer[buffer_len] = '\0';
        }
    }
    return NULL;
}

// Accept incoming connections
void* server_thread(void *arg) {
    int server_fd = *(int*)arg; // get server socket

    while (1) {
        struct sockaddr_in client_addr;
        socklen_t addr_len = sizeof(client_addr);
        int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &addr_len); // accept connection

        if (client_fd < 0) continue;

        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN); // extract ip
        int client_port = ntohs(client_addr.sin_port); // extract port
        add_peer(client_fd, client_ip, client_port);

        pthread_t tid;
        int *pclient = malloc(sizeof(int));
        *pclient = client_fd;

        pthread_create(&tid, NULL, handle_peer, pclient); // create thread for that peer
        pthread_detach(tid);
    }
}

// User input thread
void* input_thread(void *arg) {
    char buffer[BUFFER_SIZE];

    while (fgets(buffer, BUFFER_SIZE, stdin)) { // read from keyboard
        broadcast(buffer, -1); // no sender -> send to ALL
    }

    return NULL;
}

// Connect to another peer
void connect_to_peer(char *ip, int port) {
    int sock = socket(AF_INET, SOCK_STREAM, 0); // create socket

    // set up address
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &addr.sin_addr);

    if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) == 0) {
        printf("Connected to peer %s:%d\n", ip, port);

        add_peer(sock, ip, port);

        pthread_t tid;
        int *psock = malloc(sizeof(int));
        *psock = sock;

        pthread_create(&tid, NULL, handle_peer, psock);
        pthread_detach(tid);
        char hello[32];
        snprintf(hello, sizeof(hello), "Hello:%d\n", my_port);
        send(sock, hello, strlen(hello), 0);
    } else {
        perror("Connect failed");
        close(sock);
    }
}
