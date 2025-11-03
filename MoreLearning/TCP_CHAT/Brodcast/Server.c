#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>

#define PORT 5000
#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

// structure to store client info
struct Client {
    int sockfd;
    struct sockaddr_in addr;
};

// global array of clients
struct Client clients[MAX_CLIENTS];

// helper function - add new client
void add_client(int sockfd, struct sockaddr_in addr) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].sockfd == 0) {
            clients[i].sockfd = sockfd;
            clients[i].addr = addr;
            printf("Added client %d (%s:%d)\n", i, inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
            break;
        }
    }
}

// helper function - remove disconnected client
void remove_client(int sockfd) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].sockfd == sockfd) {
            close(sockfd);
            clients[i].sockfd = 0;
            break;
        }
    }
}

// helper function - broadcast message to all except sender
void broadcast(int sender, const char *msg) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].sockfd != 0 && clients[i].sockfd != sender) {
            send(clients[i].sockfd, msg, strlen(msg), 0);
        }
    }
}

int main() {
    // create server socket
    int server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0) return 1;

    // define server
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT);

    // bind and listen
    if (bind(server_sock, (struct sockaddr *)&server, sizeof(server)) < 0) return 1;
    if (listen(server_sock, 5) < 0) return 1;

    printf("Server running on port %d\n", PORT);

    // initialize clients array
    memset(clients, 0, sizeof(clients));

    // main loop
    while (1) {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(server_sock, &readfds);
        int max_sd = server_sock;

        // add client sockets to fd set
        for (int i = 0; i < MAX_CLIENTS; i++) {
            int sd = clients[i].sockfd;
            if (sd > 0) FD_SET(sd, &readfds);
            if (sd > max_sd) max_sd = sd;
        }

        // wait for activity
        select(max_sd + 1, &readfds, NULL, NULL, NULL);

        // new connection
        if (FD_ISSET(server_sock, &readfds)) {
            struct sockaddr_in client;
            socklen_t len = sizeof(client);
            int new_sock = accept(server_sock, (struct sockaddr *)&client, &len);
            if (new_sock > 0) add_client(new_sock, client);
        }

        // handle messages
        for (int i = 0; i < MAX_CLIENTS; i++) {
            int sd = clients[i].sockfd;
            if (sd > 0 && FD_ISSET(sd, &readfds)) {
                char buffer[BUFFER_SIZE];
                memset(buffer, 0, sizeof(buffer));

                ssize_t n = recv(sd, buffer, sizeof(buffer), 0);
                if (n <= 0) {
                    printf("Client %d disconnected\n", i);
                    remove_client(sd);
                } else {
                    printf("Client %d: %s", i, buffer);
                    broadcast(sd, buffer);
                }
            }
        }
    }

    close(server_sock);
    return 0;
}
