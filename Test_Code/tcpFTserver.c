// /* tcpFTserver.c - TCP File Transfer Server */
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
// ssize_t RecvLine(int socket, char *buffer, size_t maxLen);

// int main(int argc, char *argv[])
// {
//     if (argc != 2)
//     {
//         fprintf(stderr, "Usage: %s <port>\n", argv[0]);
//         return 1;
//     }

//     int serverPort = atoi(argv[1]);

//     int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
//     if (serverSocket < 0)
//         DieWithError("socket() failed");

//     int opt = 1;
//     setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

//     struct sockaddr_in serverAddr = {.sin_family = AF_INET, .sin_port = htons(serverPort), .sin_addr.s_addr = INADDR_ANY};
//     if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) DieWithError("bind() failed");

//     listen(serverSocket, 1);

//     int clientSocket = accept(serverSocket, NULL, NULL);
//     if (clientSocket < 0) DieWithError("accept() failed");

//     char command;
//     if (RecvAll(clientSocket, &command, 1) <= 0)  DieWithError("RecvAll() for command failed");

//     if (command == 'U')
//     {
//         uint32_t fileNameLengthNetworkOrder;
//         RecvAll(clientSocket, &fileNameLengthNetworkOrder, sizeof(fileNameLengthNetworkOrder));
//         uint32_t fileNameLength = ntohl(fileNameLengthNetworkOrder);

//         char *fileName = malloc(fileNameLength + 1);
//         RecvAll(clientSocket, fileName, fileNameLength);
//         fileName[fileNameLength] = 0;

//         char line[64];
//         if (RecvLine(clientSocket, line, sizeof(line)) <= 0) DieWithError("RecvLine() for size failed");

//         uint64_t fileSize = strtoull(line, NULL, 10);

//         FILE *file = fopen(fileName, "wb");
//         if (!file) DieWithError("fopen() failed");
//         free(fileName);

//         char buffer[BUFFER_SIZE];
//         uint64_t recvBytes = 0;
//         while (recvBytes < fileSize)
//         {
//             size_t bytesToReceive = (fileSize - recvBytes) > BUFFER_SIZE ? BUFFER_SIZE : (size_t)(fileSize - recvBytes);
//             ssize_t bytesRecv = RecvAll(clientSocket, buffer, bytesToReceive);
//             if (bytesRecv <= 0) DieWithError("RecvAll() for data failed");
//             fwrite(buffer, 1, bytesRecv, file);
//             recvBytes += bytesRecv;
//         }
//         fclose(file);
//         printf("Received %llu bytes\n", (unsigned long long)recvBytes);
//     }
//     else if (command == 'D')
//     {
//         uint32_t fileNameLengthNetworkOrder;
//         RecvAll(clientSocket, &fileNameLengthNetworkOrder, sizeof(fileNameLengthNetworkOrder));
//         uint32_t fileNameLength = ntohl(fileNameLengthNetworkOrder);

//         char *fileName = malloc(fileNameLength + 1);
//         RecvAll(clientSocket, fileName, fileNameLength);
//         fileName[fileNameLength] = 0;

//         FILE *file = fopen(fileName, "rb");
//         if (!file)
//         {
//             SendAll(clientSocket, "0\n", 2);
//             free(fileName);
//             goto end;
//         }

//         fseek(file, 0, SEEK_END);
//         uint64_t fileSize = ftell(file);
//         rewind(file);

//         char line[64];
//         snprintf(line, sizeof(line), "%llu\n", (unsigned long long)fileSize);
//         SendAll(clientSocket, line, strlen(line));

//         char buffer[BUFFER_SIZE];
//         size_t bytesRead;
//         while ((bytesRead = fread(buffer, 1, sizeof(buffer), file)) > 0)
//             SendAll(clientSocket, buffer, bytesRead);

//         fclose(file);
//         free(fileName);
//     }
//     else
//         DieWithError("Invalid command");

// end:
//     close(clientSocket);
//     close(serverSocket);
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

// ssize_t RecvLine(int socket, char *buffer, size_t maxLen)
// {
//     size_t pos = 0;
//     while (pos + 1 < maxLen)
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

#define PORT 12345
#define BUF  (1024*10)

static void die(const char *s){ perror(s); exit(1); }

static ssize_t recv_all(int fd, void *buf, size_t len){
    size_t t=0;
    while(t<len){
        ssize_t r = recv(fd, (char*)buf + t, len - t, 0);
        if(r<=0) return r;
        t += r;
    }
    return t;
}

int main(void){
    int srv = socket(AF_INET, SOCK_STREAM, 0); if(srv<0) die("socket");
    int opt = 1; setsockopt(srv, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    struct sockaddr_in a = { .sin_family = AF_INET, .sin_port = htons(PORT), .sin_addr.s_addr = INADDR_ANY };
    if(bind(srv,(struct sockaddr*)&a,sizeof(a))<0) die("bind");
    if(listen(srv,10)<0) die("listen");
    printf("listening %d\n", PORT);

    int c = accept(srv, NULL, NULL); if(c<0) die("accept");

    uint32_t name_len_be; if(recv_all(c,&name_len_be,sizeof(name_len_be))<=0) die("recv");
    uint32_t name_len = ntohl(name_len_be);
    if(name_len==0 || name_len>4096) die("bad name len");

    char *name = malloc(name_len+1); if(!name) die("malloc");
    if(recv_all(c,name,name_len)<=0) die("recv name"); name[name_len]=0;

    uint64_t fs_be; if(recv_all(c,&fs_be,sizeof(fs_be))<=0) die("recv size");
    uint64_t filesize = be64toh(fs_be);

    FILE *f = fopen(name,"wb"); if(!f) die("fopen");
    free(name);

    char buf[BUF];
    uint64_t got = 0;
    while(got < filesize){
        size_t want = (filesize - got) > BUF ? BUF : (size_t)(filesize - got);
        ssize_t r = recv_all(c, buf, want);
        if(r<=0) die("recv data");
        fwrite(buf,1,r,f);
        got += r;
    }
    fclose(f);
    close(c);
    close(srv);
    printf("received %llu bytes\n", (unsigned long long)filesize);
    return 0;
}


