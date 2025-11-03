/*
 * UDP Turn-Taking Chat Server
 * Simple 1-on-1 chat with turn-based messaging
 * Features: Clean I/O, proper error handling, graceful shutdown
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <signal.h>
#include <sys/time.h>
#include <errno.h>
#include <time.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define TIMEOUT_SEC 300  // 5 minutes timeout

// Global variables
int sockfd;
volatile sig_atomic_t keep_running = 1;

// Function prototypes
void signal_handler(int sig);
void print_header();
const char* get_timestamp();

// Signal handler for graceful shutdown
void signal_handler(int sig) {
    printf("\n[%s] Server shutting down...\n", get_timestamp());
    keep_running = 0;
}

// Print server header
void print_header() {
    printf("╔════════════════════════════════════════╗\n");
    printf("║   UDP Turn-Taking Chat Server          ║\n");
    printf("╚════════════════════════════════════════╝\n");
}

// Get formatted timestamp
const char* get_timestamp() {
    static char time_str[20];
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    strftime(time_str, sizeof(time_str), "%H:%M:%S", t);
    return time_str;
}

int main(int argc, char *argv[]) {
    char buffer[BUFFER_SIZE];
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);
    int port = PORT;
    int client_connected = 0;
    
    // Parse command line arguments
    if (argc > 1) {
        port = atoi(argv[1]);
        if (port <= 0 || port > 65535) {
            fprintf(stderr, "Invalid port number. Using default: %d\n", PORT);
            port = PORT;
        }
    }
    
    // Setup signal handlers
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    // Create UDP socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }
    
    // Set socket options
    int optval = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, 
                   &optval, sizeof(optval)) < 0) {
        perror("Setsockopt failed");
    }
    
    // Setup timeout for recvfrom
    struct timeval tv;
    tv.tv_sec = 1;  // 1 second timeout for non-blocking behavior
    tv.tv_usec = 0;
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, 
                   &tv, sizeof(tv)) < 0) {
        perror("Setsockopt timeout failed");
    }
    
    // Initialize server address structure
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);
    
    // Bind socket to address
    if (bind(sockfd, (const struct sockaddr *)&server_addr, 
             sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    
    print_header();
    printf("Port: %d\n", port);
    printf("Waiting for client connection...\n");
    printf("(Press Ctrl+C to exit)\n\n");
    
    // Main server loop
    while (keep_running) {
        // Receive message from client
        memset(buffer, 0, BUFFER_SIZE);
        
        ssize_t n = recvfrom(sockfd, buffer, BUFFER_SIZE - 1, 0,
                            (struct sockaddr *)&client_addr, &addr_len);
        
        if (n < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // Timeout - check if we should continue
                continue;
            }
            if (errno == EINTR) {
                // Interrupted by signal
                continue;
            }
            perror("Recvfrom failed");
            continue;
        }
        
        if (n == 0) continue;
        
        buffer[n] = '\0';
        
        // First message from client - establish connection
        if (!client_connected) {
            client_connected = 1;
            printf("[%s] Client connected from %s:%d\n",
                   get_timestamp(),
                   inet_ntoa(client_addr.sin_addr),
                   ntohs(client_addr.sin_port));
            printf("You can now start chatting!\n");
            printf("Type '/exit' to end the chat\n\n");
        }
        
        // Check for exit command
        if (strncmp(buffer, "/exit", 5) == 0) {
            printf("[%s] Client disconnected.\n", get_timestamp());
            
            // Send acknowledgment
            const char *ack = "[SERVER] Goodbye!";
            sendto(sockfd, ack, strlen(ack), 0,
                   (const struct sockaddr *)&client_addr, addr_len);
            
            client_connected = 0;
            printf("\nWaiting for new client connection...\n\n");
            continue;
        }
        
        // Display received message
        printf("[%s] Client: %s\n", get_timestamp(), buffer);
        
        // Send response
        printf("Server: ");
        fflush(stdout);
        
        if (fgets(buffer, BUFFER_SIZE, stdin) == NULL) {
            if (!keep_running) break;
            continue;
        }
        
        buffer[strcspn(buffer, "\n")] = '\0';
        
        // Check if server wants to exit
        if (strcmp(buffer, "/exit") == 0) {
            const char *goodbye = "[SERVER] Server is closing the connection. Goodbye!";
            sendto(sockfd, goodbye, strlen(goodbye), 0,
                   (const struct sockaddr *)&client_addr, addr_len);
            
            printf("[%s] Connection closed by server.\n", get_timestamp());
            client_connected = 0;
            printf("\nWaiting for new client connection...\n\n");
            continue;
        }
        
        // Send message to client
        if (sendto(sockfd, buffer, strlen(buffer), 0,
                   (const struct sockaddr *)&client_addr, addr_len) < 0) {
            perror("Sendto failed");
        }
    }
    
    // Cleanup
    close(sockfd);
    printf("[%s] Server stopped.\n", get_timestamp());
    
    return 0;
}
