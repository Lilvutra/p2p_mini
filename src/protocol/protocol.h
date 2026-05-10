#ifndef PROTOCOL_H
#define PROTOCOL_H

void broadcast(char *msg, int sender_sock);
void handle_known_hosts(int requester_sock);

#endif