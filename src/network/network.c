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
#include <time.h>

char seen_ids[1000][64]; // store 1000 message ids (passport)
int seen_head= 0; // where to write next
int msg_counter = 0;

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
        // Hello handshake
        char hello[32];
        snprintf(hello, sizeof(hello), "Hello:%d\n", my_port);
        send(sock, hello, strlen(hello), 0);
        send(sock,"/known_hosts\n", 13, 0); // send known_hosts to peer
    } else {
        perror("Connect failed");
        close(sock);
    }
}

void parse_n_connect (char *payload) {
    //printf("[debug] parse_n_connect called with : %s\n", payload);
    char copy[BUFFER_SIZE];
    strncpy(copy, payload, BUFFER_SIZE);

    char *token = strtok(copy, ",");
    while (token != NULL) {
        //printf("[debug] token: %s\n", token);
        char *colon = strchr(token, ':');
        if (colon == NULL) {
            printf("[debug] no colon found, skipping\n");
            token = strtok(NULL, ",");
            continue;
        }
        *colon = '\0';
        // ip:port
        char *ip = token;
        int port = atoi(colon+1);

        //printf("[debug] ip=%s port=%d\n", ip, port);
        //printf("[debug] already_connected=%d\n", already_connected(ip, port));

        if (already_connected(ip, port) != 1) {
            connect_to_peer(ip, port);
        }
        token = strtok(NULL, ",");
    }
}

int seen_before(char *id) {
    for (int i = 0; i <1000; i++){
        if (strcmp(seen_ids[i], id) == 0){
            return 1;
        }
    }
    return 0;
}

int mark_seen(char *id) {
    strncpy(seen_ids[seen_head], id,63);
    seen_head = (seen_head +1 ) % 1000; // ring buffer
}

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
            //printf("Received: %s\n", message);
            // forward message to others
            if (strncmp(message, "/known_hosts", 12) == 0) {
                handle_known_hosts(sock);
                //printf("known_hosts requested \n");
            } else if(strncmp(message, "Hello:", 6) == 0){
                // extract port number from "Hello:5001"
                int their_port = atoi(message +6);
                //update this peer's port in peers[]
                update_peer_port(sock, their_port);
            } else if(strncmp(message, "HOSTS:", 6) == 0){
                parse_n_connect(message+6); // skip 6 character to get the hosts number only
            } else if(strncmp(message, "MSG:", 4) == 0){ // handle message with id
                // message will be: MSG:5001-123-0|hello
                char *pipe = strchr(message+4, '|');
                if (pipe == NULL) {
                    continue;
                }

                *pipe = '\0';
                char *id = message+4; // things after : and before |
                char *text = pipe +1; // things after |

                if (seen_before(id) != 1){
                    mark_seen(id);
                    printf("MSG: %s\n", text);
                    char outgoing[BUFFER_SIZE];
                    snprintf(outgoing, sizeof(outgoing), "MSG:%s|%s\n", id, text);
                    broadcast(outgoing, sock);
                }
            }
            else{
                printf("WARNING: unknown message: %s\n ", message);
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
        buffer[strcspn(buffer,"\n")] = '\0';
        // gen id
        char id[64];
        snprintf(id, sizeof(id), "%d-%ld-%d", my_port, time(NULL), msg_counter);
        // combine to outgoing msg
        char outgoing[BUFFER_SIZE + 70];
        snprintf(outgoing, sizeof(outgoing), "MSG:%s|%s\n",id, buffer);
        mark_seen(id);
        broadcast(outgoing, -1); // no sender -> send to ALL
    }
    return NULL;
}
