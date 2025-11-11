#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <libgen.h>

#define BUFFER_SIZE 8192

void DieWithError(const char *errorMessage);
void SendFile(int sock, const char *filePath);
void ReceiveFile(int sock, const char *filePath);

int main(int argc, char *argv[]) {
    int sock;
    struct sockaddr_in servAddr;
    char *servIP;
    unsigned short servPort;

    if (argc < 5) {
        fprintf(stderr, "Usage: %s <Server IP> <Port> <SEND|RECV> <File Path> [Local Path]\n", argv[0]);
        exit(1);
    }

    servIP = argv[1];
    servPort = atoi(argv[2]);
    char *command = argv[3];
    char *filePath = argv[4];

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        DieWithError("socket() failed");
    }

    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    if (inet_pton(AF_INET, servIP, &servAddr.sin_addr) <= 0) {
        DieWithError("inet_pton() failed");
    }
    servAddr.sin_port = htons(servPort);

    if (connect(sock, (struct sockaddr *)&servAddr, sizeof(servAddr)) < 0) {
        DieWithError("connect() failed");
    }

    char buffer[BUFFER_SIZE];
    char *fileName = basename(filePath);
    snprintf(buffer, sizeof(buffer), "%s %s", command, fileName);
    if (send(sock, buffer, strlen(buffer), 0) != strlen(buffer)) {
        DieWithError("send() command failed");
    }

    ssize_t bytesRcvd = recv(sock, buffer, BUFFER_SIZE - 1, 0);
    if (bytesRcvd > 0) {
        buffer[bytesRcvd] = '\0';
        if (strcmp(buffer, "OK\n") == 0) {
            printf("Server accepted the request. Proceed with transfer? (y/n): ");
            char response = getchar();
            while (getchar() != '\n'); // Clear input buffer

            if (response == 'y' || response == 'Y') {
                send(sock, "OK\n", 3, 0);
                if (strcmp(command, "SEND") == 0) {
                    SendFile(sock, filePath);
                } else if (strcmp(command, "RECV") == 0) {
                    char *localPath = (argc > 5) ? argv[5] : fileName;
                    ReceiveFile(sock, localPath);
                }
            } else {
                send(sock, "CANCEL\n", 7, 0);
                printf("Transfer cancelled by user.\n");
            }
        } else {
            printf("Server rejected the request.\n");
        }
    } else {
        printf("No response from server.\n");
    }

    close(sock);
    return 0;
}

void SendFile(int sock, const char *filePath) {
    int file_fd = open(filePath, O_RDONLY);
    if (file_fd < 0) {
        DieWithError("open() failed for reading file");
    }

    char buffer[BUFFER_SIZE];
    ssize_t bytesRead;

    while ((bytesRead = read(file_fd, buffer, BUFFER_SIZE)) > 0) {
        if (send(sock, buffer, bytesRead, 0) != bytesRead) {
            DieWithError("send() failed");
        }
    }

    close(file_fd);
    printf("Finished sending file.\n");
}

void ReceiveFile(int sock, const char *filePath) {
    int file_fd = open(filePath, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (file_fd < 0) {
        DieWithError("open() failed for writing file");
    }

    char buffer[BUFFER_SIZE];
    ssize_t bytesRcvd;

    while ((bytesRcvd = recv(sock, buffer, BUFFER_SIZE, 0)) > 0) {
        if (write(file_fd, buffer, bytesRcvd) != bytesRcvd) {
            DieWithError("write() failed to file");
        }
    }
    
    if (bytesRcvd < 0) {
        DieWithError("recv() failed");
    }

    close(file_fd);
    printf("Finished receiving file.\n");
}

void DieWithError(const char *errorMessage) {
    perror(errorMessage);
    exit(1);
}
