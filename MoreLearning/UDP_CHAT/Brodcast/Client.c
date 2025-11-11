#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

int sockfd;
struct sockaddr_in server;
int running = 1;

void *receive_messages(void *arg) {
    (void)arg;
    char buffer[256];
    while (running) {
        memset(buffer, 0, sizeof(buffer));
        socklen_t len = sizeof(server);
        if (recvfrom(sockfd, buffer, sizeof(buffer), 0, 
                     (struct sockaddr *)&server, &len) <= 0) break;
        printf("\nMsg: %s", buffer);
        printf("You: ");
        fflush(stdout);
    }
    return NULL;
}

void *send_messages(void *arg) {
    (void)arg;
    char buffer[256];
    while (running) {
        printf("You: ");
        fflush(stdout);
        fgets(buffer, sizeof(buffer), stdin);
        sendto(sockfd, buffer, strlen(buffer), 0, 
               (struct sockaddr *)&server, sizeof(server));
    }
    return NULL;
}

int main() {
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_port = htons(8080);

    pthread_t recv_thread, send_thread;
    pthread_create(&recv_thread, NULL, receive_messages, NULL);
    pthread_create(&send_thread, NULL, send_messages, NULL);

    pthread_join(send_thread, NULL);
    pthread_join(recv_thread, NULL);

    close(sockfd);
    return 0;
}
