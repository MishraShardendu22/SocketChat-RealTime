#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>

#define MAX_CLIENTS 10

int clients[MAX_CLIENTS];

void broadcast(int sender, const char *msg) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i] != 0 && clients[i] != sender) {
            send(clients[i], msg, strlen(msg), 0);
        }
    }
}

int main() {
    int server_sock = socket(AF_INET, SOCK_STREAM, 0);
    
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(5000);

    bind(server_sock, (struct sockaddr *)&server, sizeof(server));
    listen(server_sock, 5);

    memset(clients, 0, sizeof(clients));

    while (1) {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(server_sock, &readfds);
        int max_sd = server_sock;

        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i] > 0) FD_SET(clients[i], &readfds);
            if (clients[i] > max_sd) max_sd = clients[i];
        }

        select(max_sd + 1, &readfds, NULL, NULL, NULL);

        if (FD_ISSET(server_sock, &readfds)) {
            struct sockaddr_in client;
            socklen_t len = sizeof(client);
            int new_sock = accept(server_sock, (struct sockaddr *)&client, &len);
            for (int i = 0; i < MAX_CLIENTS; i++) {
                if (clients[i] == 0) {
                    clients[i] = new_sock;
                    break;
                }
            }
        }

        for (int i = 0; i < MAX_CLIENTS; i++) {
            int sd = clients[i];
            if (sd > 0 && FD_ISSET(sd, &readfds)) {
                char buffer[256];
                memset(buffer, 0, sizeof(buffer));
                
                if (recv(sd, buffer, sizeof(buffer), 0) <= 0) {
                    close(sd);
                    clients[i] = 0;
                } else {
                    broadcast(sd, buffer);
                }
            }
        }
    }
    
    close(server_sock);
    return 0;
}
