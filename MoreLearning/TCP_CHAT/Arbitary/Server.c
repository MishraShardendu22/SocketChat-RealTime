#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

int client_sock;
int running = 1;

void *receive_messages(void *arg) {
    (void)arg; // Suppress unused parameter warning
    char buffer[256];
    while (running) {
        memset(buffer, 0, sizeof(buffer));
        int bytes_received = recv(client_sock, buffer, sizeof(buffer), 0);
        if (bytes_received <= 0) {
            running = 0;
            break;
        }
        printf("\rClient: %s", buffer);
        printf("You: ");
        fflush(stdout);
    }
    return NULL;
}

void *send_messages(void *arg) {
    (void)arg; // Suppress unused parameter warning
    char buffer[256];
    while (running) {
        printf("You: ");
        fflush(stdout);
        fgets(buffer, sizeof(buffer), stdin);
        send(client_sock, buffer, strlen(buffer), 0);
    }
    return NULL;
}

int main() {
    int server_sock = socket(AF_INET, SOCK_STREAM, 0);
    
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(5000);

    bind(server_sock, (struct sockaddr *)&server, sizeof(server));
    listen(server_sock, 1);

    struct sockaddr_in client;
    socklen_t cli_len = sizeof(client);
    client_sock = accept(server_sock, (struct sockaddr *)&client, &cli_len);
 
    pthread_t recv_thread, send_thread;
    pthread_create(&recv_thread, NULL, receive_messages, NULL);
    pthread_create(&send_thread, NULL, send_messages, NULL);

    pthread_join(send_thread, NULL);
    pthread_join(recv_thread, NULL);

    close(client_sock);
    close(server_sock);
    return 0;
}
