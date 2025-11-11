// UDP Server Code ()
// The server waits for a client's message, prints it, then prompts for a reply to send back. The recvfrom() call provides the client's address, which is then used in the subsequent sendto() call.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 12000
#define MAX_BUFFER_SIZE 1024

// This function provides a basic outline of the server's operation.
void run_server() {
    int serverSocket;
    struct sockaddr_in serverAddr, clientAddr;
    char buffer[MAX_BUFFER_SIZE];
    socklen_t clientAddrLen = sizeof(clientAddr);

    // 1. Create UDP socket (SOCK_DGRAM for UDP) [8, 9]
    serverSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (serverSocket < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&serverAddr, 0, sizeof(serverAddr));
    
    // 2. Configure server address structure [20]
    serverAddr.sin_family = AF_INET; // IPv4 [21]
    serverAddr.sin_addr.s_addr = INADDR_ANY; // Listen on all interfaces
    serverAddr.sin_port = htons(PORT); // Port in network byte order [22]

    // 3. Bind the socket to the server address and port [23, 24]
    if (bind(serverSocket, (const struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("bind failed");
        close(serverSocket);
        exit(EXIT_FAILURE);
    }

    printf("UDP server is listening on port %d...\n", PORT);

    // 4. Wait for the initial message from a client
    ssize_t n = recvfrom(serverSocket, (char *)buffer, MAX_BUFFER_SIZE, 0,
                         (struct sockaddr *) &clientAddr, &clientAddrLen);
    buffer[n] = '\0';
    printf("Client: %s\n", buffer);

    // 5. Loop for alternating communication
    while (1) {
        // Get server's message and send it to the client
        printf("Server: ");
        fgets(buffer, MAX_BUFFER_SIZE, stdin);
        sendto(serverSocket, buffer, strlen(buffer), 0,
               (const struct sockaddr *) &clientAddr, clientAddrLen);

        // Check if the server wants to exit
        if (strncmp(buffer, "exit", 4) == 0) {
            printf("Server is exiting.\n");
            break;
        }

        // Wait for client's reply
        n = recvfrom(serverSocket, (char *)buffer, MAX_BUFFER_SIZE, 0,
                     (struct sockaddr *) &clientAddr, &clientAddrLen);
        buffer[n] = '\0';
        printf("Client: %s", buffer);

        // Check if the client wants to exit
        if (strncmp(buffer, "exit", 4) == 0) {
            printf("Client has exited. Server is shutting down.\n");
            break;
        }
    }

    // 6. Close the socket [25]
    close(serverSocket);
}

int main() {
    run_server();
    return 0;
}
