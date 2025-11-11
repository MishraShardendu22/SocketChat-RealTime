#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include "DieWithError.h"

#define MAXPENDING 5
#define MAX_CLIENTS 10

void *ThreadMain(void *arg);
void HandleTCPClient(int clntSocket);
int CreateTCPServerSocket(unsigned short port);
int AcceptTCPConnection(int servSock);

struct ThreadArgs {
    int clntSock;
};

// Globals for client management
int client_sockets[MAX_CLIENTS];
pthread_mutex_t client_mutex;

int main(int argc, char *argv[]) {
    int servSock;
    int clntSock;
    unsigned short echoServPort;
    pthread_t threadID;
    struct ThreadArgs *threadArgs;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <Server Port>\n", argv[0]);
        exit(1);
    }

    echoServPort = atoi(argv[1]);

    // Initialize client sockets array
    for (int i = 0; i < MAX_CLIENTS; i++) {
        client_sockets[i] = 0;
    }
    pthread_mutex_init(&client_mutex, NULL);

    servSock = CreateTCPServerSocket(echoServPort);

    printf("Arbitrary TCP Chat Server started on port %d\n", echoServPort);

    for (;;) {
        clntSock = AcceptTCPConnection(servSock);

        if ((threadArgs = (struct ThreadArgs *) malloc(sizeof(struct ThreadArgs))) == NULL)
            DieWithError("malloc() failed");
        threadArgs->clntSock = clntSock;

        if (pthread_create(&threadID, NULL, ThreadMain, (void *) threadArgs) != 0)
            DieWithError("pthread_create() failed");

        printf("Created thread %ld for client connection\n", (long)threadID);
    }
}

void *ThreadMain(void *threadArgs) {
    int clntSock;

    pthread_detach(pthread_self());

    clntSock = ((struct ThreadArgs *) threadArgs)->clntSock;
    free(threadArgs);

    HandleTCPClient(clntSock);

    return (NULL);
}

void broadcast(char *message, int sender_sock) {
    pthread_mutex_lock(&client_mutex);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (client_sockets[i] != 0 && client_sockets[i] != sender_sock) {
            if (send(client_sockets[i], message, strlen(message), 0) < 0) {
                DieWithError("send() failed during broadcast");
            }
        }
    }
    pthread_mutex_unlock(&client_mutex);
}

void HandleTCPClient(int clntSocket) {
    char buffer[256];
    int recvMsgSize;

    // Add client to list
    pthread_mutex_lock(&client_mutex);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (client_sockets[i] == 0) {
            client_sockets[i] = clntSocket; 
            break;
        }
    }
    pthread_mutex_unlock(&client_mutex);

    while ((recvMsgSize = recv(clntSocket, buffer, 256, 0)) > 0) {
        buffer[recvMsgSize] = '\0';
        printf("Received from client %d: %s", clntSocket, buffer);
        broadcast(buffer, clntSocket);
    }

    // Remove client from list
    pthread_mutex_lock(&client_mutex);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (client_sockets[i] == clntSocket) {
            client_sockets[i] = 0;
            break;
        }
    }
    pthread_mutex_unlock(&client_mutex);

    close(clntSocket);
    printf("Thread: Client connection closed for socket %d\n", clntSocket);
}

int CreateTCPServerSocket(unsigned short port) {
    int sock;
    struct sockaddr_in echoServAddr;

    if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        DieWithError("socket() failed");

    memset(&echoServAddr, 0, sizeof(echoServAddr));
    echoServAddr.sin_family = AF_INET;
    echoServAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    echoServAddr.sin_port = htons(port);

    if (bind(sock, (struct sockaddr *) &echoServAddr, sizeof(echoServAddr)) < 0)
        DieWithError("bind() failed");

    if (listen(sock, MAXPENDING) < 0)
        DieWithError("listen() failed");

    return sock;
}

int AcceptTCPConnection(int servSock) {
    int clntSock;
    struct sockaddr_in echoClntAddr;
    unsigned int clntLen;

    clntLen = sizeof(echoClntAddr);

    if ((clntSock = accept(servSock, (struct sockaddr *) &echoClntAddr, &clntLen)) < 0)
        DieWithError("accept() failed");

    printf("Handling client %s\n", inet_ntoa(echoClntAddr.sin_addr));

    return clntSock;
}