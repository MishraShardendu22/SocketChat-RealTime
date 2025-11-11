#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>

int main() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(5000);
    server.sin_addr.s_addr = inet_addr("127.0.0.1");

    connect(sock, (struct sockaddr*)&server, sizeof(server));

    fd_set fds;
    char buffer[256];

    while (1) {
        FD_ZERO(&fds);
        FD_SET(sock, &fds);
        FD_SET(0, &fds);

        select(sock + 1, &fds, NULL, NULL, NULL);

        if (FD_ISSET(sock, &fds)) {
            memset(buffer, 0, sizeof(buffer));
            if (recv(sock, buffer, sizeof(buffer), 0) <= 0) break;
            printf("Msg: %s", buffer);
        }

        if (FD_ISSET(0, &fds)) {
            memset(buffer, 0, sizeof(buffer));
            fgets(buffer, sizeof(buffer), stdin);
            send(sock, buffer, strlen(buffer), 0);
        }
    }

    close(sock);
    return 0;
}
