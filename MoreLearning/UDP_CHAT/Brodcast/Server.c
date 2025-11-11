#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define MAX_CLIENTS 10

struct sockaddr_in clients[MAX_CLIENTS];
int client_count = 0;
int sockfd;

void broadcast(struct sockaddr_in *sender, const char *msg) {
    for (int i = 0; i < client_count; i++) {
        if (clients[i].sin_port != sender->sin_port || 
            clients[i].sin_addr.s_addr != sender->sin_addr.s_addr) {
            sendto(sockfd, msg, strlen(msg), 0, 
                   (struct sockaddr *)&clients[i], sizeof(clients[i]));
        }
    }
}

int main() {
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(8080);

    bind(sockfd, (struct sockaddr *)&server, sizeof(server));

    while (1) {
        char buffer[256];
        struct sockaddr_in client;
        socklen_t len = sizeof(client);
        
        memset(buffer, 0, sizeof(buffer));
        recvfrom(sockfd, buffer, sizeof(buffer), 0, 
                 (struct sockaddr *)&client, &len);

        int found = 0;
        for (int i = 0; i < client_count; i++) {
            if (clients[i].sin_port == client.sin_port && 
                clients[i].sin_addr.s_addr == client.sin_addr.s_addr) {
                found = 1;
                break;
            }
        }
        
        if (!found && client_count < MAX_CLIENTS) {
            clients[client_count++] = client;
        }

        broadcast(&client, buffer);
    }

    close(sockfd);
    return 0;
}
