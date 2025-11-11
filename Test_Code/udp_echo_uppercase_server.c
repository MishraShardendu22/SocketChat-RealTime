// UDP Server Code ()
// The server waits for a datagram, processes the message, and sends a reply to the client's address.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ctype.h>

#define PORT 12000
#define MAX_BUFFER_SIZE 2048

int main() {
    int serverSocket;
    struct sockaddr_in serverAddr, clientAddr;
    char buffer[MAX_BUFFER_SIZE];
    socklen_t clientAddrLen = sizeof(clientAddr);

    // 1. Create UDP socket (SOCK_DGRAM for UDP) [12]
    serverSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (serverSocket < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&serverAddr, 0, sizeof(serverAddr));
    
    // 2. Configure server address structure [13]
    serverAddr.sin_family = AF_INET; // IPv4
    serverAddr.sin_addr.s_addr = INADDR_ANY; // Listen on all available interfaces
    serverAddr.sin_port = htons(PORT); // Port in network byte order [14, 15]

    // 3. Bind the socket to the server address and port [16, 17]
    if (bind(serverSocket, (const struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("bind failed");
        close(serverSocket);
        exit(EXIT_FAILURE);
    }

    printf("UDP server is ready to receive on port %d\n", PORT);

    // Loop forever to handle multiple clients sequentially [6]
    while (1) {
        // 4. Receive a message from a client (blocking call) [2]
        ssize_t n = recvfrom(serverSocket, (char *)buffer, MAX_BUFFER_SIZE, 0, 
                             (struct sockaddr *) &clientAddr, &clientAddrLen);
        
        if (n < 0) {
            perror("recvfrom failed");
            continue; // Continue to the next iteration
        }

        buffer[n] = '\0'; // Null-terminate the string
        printf("Received from %s:%d -> Message: %s\n", 
               inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port), buffer);
        
        // 5. Process the data (convert to uppercase) [5]
        for (int i = 0; i < n; i++) {
            buffer[i] = toupper(buffer[i]);
        }

        // 6. Send the modified message back to the client [2]
        // clientAddress (IP and port) was obtained from recvfrom() [5]
        sendto(serverSocket, buffer, n, 0, 
               (const struct sockaddr *) &clientAddr, clientAddrLen);
        
        printf("Sent reply to %s:%d -> Message: %s\n", 
               inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port), buffer);
    }

    // This part is unreachable in this example, but good practice
    close(serverSocket); 
    
    return 0;
}
