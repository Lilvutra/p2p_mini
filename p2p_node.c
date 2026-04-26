#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>

#define MAX_PEERS 100
#define BUFFER_SIZE 1024

// Shared memory
int peers[MAX_PEERS]; // list of connected sockets
int peer_count = 0;  // number of peers

pthread_mutex_t peers_mutex; // protects access

// Direct TCP P2P connection
// Real-time messaging and multi-peer broadcasting

// Current to-do:
// Look into the code more clearly to possible detect fault logic

// Further lookinto:
// - Test if we can prevent message loop: 
    // + Since now nodes are connected as a tree base network 5000->5001->5002-> there is no loop. 
    // + When we run same port(connected to a same port), it still runs(can run ./node 5001 127.0.0.1 5000 in 2 terminals without conflict) -> Check if bind() is silently failing
// - If not, message identity + memory so that each message is processed once




// Add peer safely
void add_peer(int sock) {
    pthread_mutex_lock(&peers_mutex);

    if (peer_count < MAX_PEERS) {
        peers[peer_count++] = sock;
        printf("New peer added. Total: %d\n", peer_count);
    }

    pthread_mutex_unlock(&peers_mutex);
}


void remove_peer(int sock) {
    pthread_mutex_lock(&peers_mutex);

    for (int i = 0; i < peer_count; i++) {
        if (peers[i] == sock) {
            peers[i] = peers[peer_count - 1];
            peer_count--;
            break;
        }
    }

    pthread_mutex_unlock(&peers_mutex);
}


// Broadcast message to all peers
void broadcast(char *msg, int sender_sock) {
    pthread_mutex_lock(&peers_mutex);

    for (int i = 0; i < peer_count; i++) {
        int sock = peers[i];

        if (sock != sender_sock) { // except sender
            send(sock, msg, strlen(msg), 0); // send raw message
        }
    }

    pthread_mutex_unlock(&peers_mutex);
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
            printf("Received: %s\n", message);
            // forward message to others
            broadcast(buffer, sock);

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
        int client_fd = accept(server_fd, NULL, NULL); // accept connection

        if (client_fd < 0) continue;

        add_peer(client_fd);

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

        add_peer(sock);

        pthread_t tid;
        int *psock = malloc(sizeof(int));
        *psock = sock;

        pthread_create(&tid, NULL, handle_peer, psock);
        pthread_detach(tid);
    } else {
        perror("Connect failed");
        close(sock);
    }
}

// MAIN
int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <port> [peer_ip peer_port]\n", argv[0]);
        return 1;
    }

    int port = atoi(argv[1]);

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