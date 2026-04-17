#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>

#define MAX_PEERS 100
#define BUFFER_SIZE 1024

int peers[MAX_PEERS];
int peer_count = 0;

pthread_mutex_t peers_mutex;

// Direct TCP P2P connection
// Real-time messaging and multi=peer broadcasting

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

        if (sock != sender_sock) {
            send(sock, msg, strlen(msg), 0);
        }
    }

    pthread_mutex_unlock(&peers_mutex);
}

// Handle incoming messages
void* handle_peer(void *arg) {
    int sock = *(int*)arg;
    free(arg);

    char buffer[BUFFER_SIZE];

    while (1) {
        int bytes = recv(sock, buffer, BUFFER_SIZE - 1, 0);

        if (bytes <= 0) {
            printf("Peer disconnected\n");
            close(sock);
            remove_peer(sock);
            break;
        }

        buffer[bytes] = '\0';
        printf("Received: %s", buffer);

        // forward message to others
        broadcast(buffer, sock);
    }

    return NULL;
}

// Accept incoming connections
void* server_thread(void *arg) {
    int server_fd = *(int*)arg;

    while (1) {
        int client_fd = accept(server_fd, NULL, NULL);

        if (client_fd < 0) continue;

        add_peer(client_fd);

        pthread_t tid;
        int *pclient = malloc(sizeof(int));
        *pclient = client_fd;

        pthread_create(&tid, NULL, handle_peer, pclient);
        pthread_detach(tid);
    }
}

// User input thread
void* input_thread(void *arg) {
    char buffer[BUFFER_SIZE];

    while (fgets(buffer, BUFFER_SIZE, stdin)) {
        broadcast(buffer, -1);
    }

    return NULL;
}

// Connect to another peer
void connect_to_peer(char *ip, int port) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);

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
    int server_fd = socket(AF_INET, SOCK_STREAM, 0); // 

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr));
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