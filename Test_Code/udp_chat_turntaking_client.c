// UDP Client Code ()
// The client initiates the conversation by sending the first message. It then enters a loop where it waits for the server's reply before prompting the user for the next message to send.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define SERVER_IP "127.0.0.1" // Use localhost
#define PORT 12000
#define MAX_BUFFER_SIZE 1024

// This function provides a basic outline of the client's operation.
void run_client() {
    int clientSocket;
    struct sockaddr_in serverAddr;
    char buffer[MAX_BUFFER_SIZE];

    // 1. Create UDP socket [8, 19] 
    clientSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (clientSocket < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&serverAddr, 0, sizeof(serverAddr));
    
    // 2. Configure server address structure
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT); // Server's port number
    serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP);

    // 3. Loop for alternating communication
    while (1) {
        // Get client's message and send it
        printf("Client: ");
        fgets(buffer, MAX_BUFFER_SIZE, stdin);
        sendto(clientSocket, buffer, strlen(buffer), 0,
               (const struct sockaddr *) &serverAddr, sizeof(serverAddr));

        // Check if the client wants to exit
        if (strncmp(buffer, "exit", 4) == 0) {
            printf("Client is exiting.\n");
            break;
        }

        // Wait for server's reply
        socklen_t serverAddrLen = sizeof(serverAddr);
        ssize_t n = recvfrom(clientSocket, (char *)buffer, MAX_BUFFER_SIZE, 0,
                             (struct sockaddr *) &serverAddr, &serverAddrLen);
        buffer[n] = '\0';
        printf("Server: %s", buffer);

        // Check if the server wants to exit
        if (strncmp(buffer, "exit", 4) == 0) {
            printf("Server has exited. Client is shutting down.\n");
            break;
        }
    }

    // 4. Close the socket
    close(clientSocket);
}

int main() {
    run_client();
    return 0;
}
