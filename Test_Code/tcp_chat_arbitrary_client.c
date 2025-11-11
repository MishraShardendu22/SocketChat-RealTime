#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include "DieWithError.h"

#define RCVBUFSIZE 256

void *send_thread(void *arg) {
    int sock = *(int *)arg;
    char buffer[RCVBUFSIZE];

    while (fgets(buffer, sizeof(buffer), stdin) != NULL) {
        if (send(sock, buffer, strlen(buffer), 0) < 0) {
            DieWithError("send() failed");
        }
    }
    return NULL;
}

void *receive_thread(void *arg) {
    int sock = *(int *)arg;
    char buffer[RCVBUFSIZE];
    int bytesRcvd;

    while ((bytesRcvd = recv(sock, buffer, RCVBUFSIZE - 1, 0)) > 0) {
        buffer[bytesRcvd] = '\0';
        printf("Received: %s", buffer);
    }

    if (bytesRcvd == 0) {
        printf("Connection closed by server.\n");
    } else {
        DieWithError("recv() failed");
    }
    
    close(sock);
    exit(0); // Exit the client when the server closes the connection

    return NULL;
}

int main(int argc, char *argv[]) {
    int sock;
    struct sockaddr_in servAddr;
    unsigned short servPort;
    char *servIP;

    if (argc != 3) {
        fprintf(stderr, "Usage: %s <Server IP> <Server Port>\n", argv[0]);
        exit(1);
    }

    servIP = argv[1];
    servPort = atoi(argv[2]);

    if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        DieWithError("socket() failed");

    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = inet_addr(servIP);
    servAddr.sin_port = htons(servPort);

    if (connect(sock, (struct sockaddr *) &servAddr, sizeof(servAddr)) < 0)
        DieWithError("connect() failed");

    printf("Connected to server. You can start sending messages.\n");

    pthread_t send_tid, recv_tid;

    if (pthread_create(&send_tid, NULL, send_thread, &sock) != 0) {
        DieWithError("pthread_create() for send_thread failed");
    }

    if (pthread_create(&recv_tid, NULL, receive_thread, &sock) != 0) {
        DieWithError("pthread_create() for receive_thread failed");
    }

    pthread_join(send_tid, NULL);
    pthread_join(recv_tid, NULL);

    close(sock);
    return 0;
}