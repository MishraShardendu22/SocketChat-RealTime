// /* tcpFTclient.c - TCP File Transfer Client */
// #include <stdio.h>
// #include <stdlib.h>
// #include <stdint.h>
// #include <string.h>
// #include <unistd.h>
// #include <arpa/inet.h>
// #include <sys/socket.h>
// #include "DieWithError.h"

// #define BUFFER_SIZE (16 * 1024)

// ssize_t SendAll(int socket, const void *buffer, size_t length);
// ssize_t RecvAll(int socket, void *buffer, size_t length);
// ssize_t RecvLine(int socket, char *buffer, size_t maxLength);

// int main(int argc, char *argv[])
// {
//     if (argc != 5)
//     {
//         fprintf(stderr, "Usage: %s <host> <port> upload|download <file>\n", argv[0]);
//         return 1;
//     }

//     const char *serverIP = argv[1];
//     int serverPort = atoi(argv[2]);
//     const char *mode = argv[3];
//     const char *filePath = argv[4];

//     int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
//     if (clientSocket < 0) DieWithError("socket() failed");

//     struct sockaddr_in serverAddr = {.sin_family = AF_INET, .sin_port = htons(serverPort)};
//     inet_pton(AF_INET, serverIP, &serverAddr.sin_addr);

//     if (connect(clientSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) DieWithError("connect() failed");

//     if (strcmp(mode, "upload") == 0)
//     {
//         char command = 'U';
//         SendAll(clientSocket, &command, 1);

//         FILE *file = fopen(filePath, "rb");
//         if (!file) DieWithError("fopen() failed");

//         fseek(file, 0, SEEK_END);
//         uint64_t fileSize = ftell(file);
//         uint32_t fileNameLength = strlen(filePath);
//         uint32_t fileNameLengthNetworkOrder = htonl(fileNameLength);
//         SendAll(clientSocket, &fileNameLengthNetworkOrder, sizeof(fileNameLengthNetworkOrder));
//         SendAll(clientSocket, filePath, fileNameLength);

//         char line[64];
//         snprintf(line, sizeof(line), "%llu\n", (unsigned long long)fileSize);
//         SendAll(clientSocket, line, strlen(line));

//         char buffer[BUFFER_SIZE];
//         size_t bytesRead;
//         while ((bytesRead = fread(buffer, 1, sizeof(buffer), file)) > 0)
//             SendAll(clientSocket, buffer, bytesRead);

//         fclose(file);
//         printf("Uploaded %llu bytes\n", (unsigned long long)fileSize);
//     }
//     else
//     {
//         char command = 'D';
//         SendAll(clientSocket, &command, 1);

//         uint32_t fileNameLength = strlen(filePath);
//         uint32_t fileNameLengthNetworkOrder = htonl(fileNameLength);
//         SendAll(clientSocket, &fileNameLengthNetworkOrder, sizeof(fileNameLengthNetworkOrder));
//         SendAll(clientSocket, filePath, fileNameLength);

//         char line[64];
//         if (RecvLine(clientSocket, line, sizeof(line)) <= 0) DieWithError("RecvLine() failed for size");

//         uint64_t fileSize = strtoull(line, NULL, 10);
//         if (fileSize == 0)
//         {
//             fprintf(stderr, "Server: File not found or is empty\n");
//             close(clientSocket);
//             return 1;
//         }

//         FILE *outputFile = fopen(filePath, "wb");
//         if (!outputFile) DieWithError("fopen() for writing failed");

//         char buffer[BUFFER_SIZE];
//         uint64_t recvBytes = 0;
//         while (recvBytes < fileSize)
//         {
//             size_t bytesToReceive = (fileSize - recvBytes) > BUFFER_SIZE ? BUFFER_SIZE : (size_t)(fileSize - recvBytes);
//             ssize_t bytesRecv = RecvAll(clientSocket, buffer, bytesToReceive);
//             if (bytesRecv <= 0) DieWithError("RecvAll() failed");

//             fwrite(buffer, 1, bytesRecv, outputFile);
//             recvBytes += bytesRecv;
//         }
//         fclose(outputFile);
//         printf("Downloaded %llu bytes\n", (unsigned long long)recvBytes);
//     }
//     close(clientSocket);
//     return 0;
// }

// ssize_t SendAll(int socket, const void *buffer, size_t length)
// {
//     size_t totalSent = 0;
//     while (totalSent < length)
//     {
//         ssize_t bytesSent = send(socket, (const char *)buffer + totalSent, length - totalSent, 0);
//         if (bytesSent <= 0) return bytesSent;
//         totalSent += bytesSent;
//     }
//     return totalSent;
// }

// ssize_t RecvAll(int socket, void *buffer, size_t length)
// {
//     size_t totalRecv = 0;
//     while (totalRecv < length)
//     {
//         ssize_t bytesRecv = recv(socket, (char *)buffer + totalRecv, length - totalRecv, 0);
//         if (bytesRecv <= 0) return bytesRecv;
//         totalRecv += bytesRecv;
//     }
//     return totalRecv;
// }

// ssize_t RecvLine(int socket, char *buffer, size_t maxLength)
// {
//     size_t pos = 0;
//     while (pos + 1 < maxLength)
//     {
//         char c;
//         ssize_t bytesRecv = recv(socket, &c, 1, 0);
//         if (bytesRecv <= 0) return bytesRecv;
//         buffer[pos++] = c;
//         if (c == '\n') break;
//     }
//     buffer[pos] = 0;
//     return pos;
// }


#define _GNU_SOURCE
#include <arpa/inet.h>
#include <machine/endian.h>
#include <netinet/in.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define BUF  (1024*10)

static void die(const char *s){ perror(s); exit(1); }

static ssize_t send_all(int fd, const void *buf, size_t len){
    size_t t=0;
    while(t<len){
        ssize_t s = send(fd, (const char*)buf + t, len - t, 0);
        if(s<=0) return s;
        t += s;
    }
    return t;
}

int main(int argc, char **argv){
    if(argc!=4){ fprintf(stderr,"Usage: %s <host> <port> <file>\n",argv[0]); return 1; }

    const char *host = argv[1]; int port = atoi(argv[2]); const char *file = argv[3];

    FILE *f = fopen(file,"rb"); if(!f) die("fopen");
    fseek(f,0,SEEK_END); uint64_t filesize = ftell(f); rewind(f);

    int sfd = socket(AF_INET, SOCK_STREAM, 0); if(sfd<0) die("socket");
    struct sockaddr_in a; memset(&a,0,sizeof(a));
    a.sin_family = AF_INET; a.sin_port = htons(port);
    if(inet_pton(AF_INET, host, &a.sin_addr) <= 0) die("inet_pton");
    if(connect(sfd,(struct sockaddr*)&a,sizeof(a))<0) die("connect");

    uint32_t name_len = strlen(file);
    uint32_t name_len_be = htonl(name_len);
    if(send_all(sfd,&name_len_be,sizeof(name_len_be))!=sizeof(name_len_be)) die("send");
    if(send_all(sfd,file,name_len)!=(ssize_t)name_len) die("send name");

    uint64_t fs_be = htobe64(filesize);
    if(send_all(sfd,&fs_be,sizeof(fs_be))!=sizeof(fs_be)) die("send size");

    char buf[BUF]; size_t n;
    while((n=fread(buf,1,sizeof(buf),f))>0){
        if(send_all(sfd,buf,n)!=(ssize_t)n) die("send data");
    }

    fclose(f);
    close(sfd);
    printf("sent %llu bytes\n",(unsigned long long)filesize);
    return 0;
}


