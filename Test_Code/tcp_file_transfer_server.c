#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>

#define DEFAULT_PORT 8080
#define BUFFER_SIZE 8192
#define MAX_PENDING 5

void DieWithError(const char *errorMessage);
void HandleTCPClient(int clntSocket, const char *clientIP);
void ReceiveFile(int clntSocket, const char *fileName);
void SendFile(int clntSocket, const char *fileName);

int main(int argc, char *argv[]) {
    int servSock, clntSock;
    struct sockaddr_in servAddr, clntAddr;
    unsigned int clntLen;
    pid_t processID;
    unsigned short servPort = DEFAULT_PORT;

    if (argc > 2) {
        fprintf(stderr, "Usage: %s [Port]\n", argv[0]);
        exit(1);
    }

    if (argc == 2) {
        servPort = atoi(argv[1]);
    }

    servSock = socket(AF_INET, SOCK_STREAM, 0);
    if (servSock < 0) {
        DieWithError("socket() failed");
    }

    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servAddr.sin_port = htons(servPort);

    if (bind(servSock, (struct sockaddr *)&servAddr, sizeof(servAddr)) < 0) {
        DieWithError("bind() failed");
    }

    if (listen(servSock, MAX_PENDING) < 0) {
        DieWithError("listen() failed");
    }

    printf("Server listening on port %d\n", servPort);

    for (;;) {
        clntLen = sizeof(clntAddr);
        clntSock = accept(servSock, (struct sockaddr *)&clntAddr, &clntLen);
        if (clntSock < 0) {
            DieWithError("accept() failed");
        }

        char *clientIP = inet_ntoa(clntAddr.sin_addr);
        printf("Handling client %s\n", clientIP);

        processID = fork();
        if (processID < 0) {
            DieWithError("fork() failed");
        } else if (processID == 0) {
            close(servSock);
            HandleTCPClient(clntSock, clientIP);
            exit(0);
        } else {
            close(clntSock);
        }
    }

    return 0;
}

void HandleTCPClient(int clntSocket, const char *clientIP) {
    char buffer[BUFFER_SIZE];
    ssize_t bytesRcvd;

    bytesRcvd = recv(clntSocket, buffer, BUFFER_SIZE - 1, 0);
    if (bytesRcvd <= 0) {
        close(clntSocket);
        return;
    }
    buffer[bytesRcvd] = '\0';

    char command[10];
    char fileName[256];
    sscanf(buffer, "%s %s", command, fileName);

    printf("Client %s wants to %s %s. Accept? (y/n): ", clientIP, command, fileName);
    char response = getchar();
    while (getchar() != '\n'); // Clear input buffer

    if (response == 'y' || response == 'Y') {
        send(clntSocket, "OK\n", 3, 0);
        bytesRcvd = recv(clntSocket, buffer, BUFFER_SIZE - 1, 0);
        if (bytesRcvd > 0) {
            buffer[bytesRcvd] = '\0';
            if (strcmp(buffer, "OK\n") == 0) {
                if (strcmp(command, "SEND") == 0) {
                    ReceiveFile(clntSocket, fileName);
                } else if (strcmp(command, "RECV") == 0) {
                    SendFile(clntSocket, fileName);
                }
            }
        }
    } else {
        send(clntSocket, "REJECTED\n", 9, 0);
    }

    close(clntSocket);
}

void ReceiveFile(int clntSocket, const char *fileName) {
    int file_fd = open(fileName, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (file_fd < 0) {
        DieWithError("open() failed for writing file");
    }

    char buffer[BUFFER_SIZE];
    ssize_t bytesRcvd;

    while ((bytesRcvd = recv(clntSocket, buffer, BUFFER_SIZE, 0)) > 0) {
        if (write(file_fd, buffer, bytesRcvd) != bytesRcvd) {
            DieWithError("write() failed to file");
        }
    }

    close(file_fd);
    printf("Finished receiving file: %s\n", fileName);
}

void SendFile(int clntSocket, const char *fileName) {
    int file_fd = open(fileName, O_RDONLY);
    if (file_fd < 0) {
        char *errMsg = "Error: File not found or cannot be opened.";
        send(clntSocket, errMsg, strlen(errMsg), 0);
        perror("open() failed for reading file");
        return;
    }

    char buffer[BUFFER_SIZE];
    ssize_t bytesRead;

    while ((bytesRead = read(file_fd, buffer, BUFFER_SIZE)) > 0) {
        if (send(clntSocket, buffer, bytesRead, 0) != bytesRead) {
            DieWithError("send() failed");
        }
    }

    close(file_fd);
    printf("Finished sending file: %s\n", fileName);
}

void DieWithError(const char *errorMessage) {
    perror(errorMessage);
    exit(1);
}
