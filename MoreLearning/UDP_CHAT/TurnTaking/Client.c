/*
 * UDP Turn-Taking Chat Client
 * Simple 1-on-1 chat client with turn-based messaging
 * Features: Clean I/O, timeout handling, graceful disconnect
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <signal.h>
#include <errno.h>
#include <time.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define RECV_TIMEOUT_SEC 60  // 60 seconds timeout

// Global variables
int sockfd;
volatile sig_atomic_t keep_running = 1;

// Function prototypes
void signal_handler(int sig);
void print_header();
const char* get_timestamp();

// Signal handler for graceful shutdown
void signal_handler(int sig) {
    printf("\n[%s] Client shutting down...\n", get_timestamp());
    keep_running = 0;
}

// Print client header
void print_header() {
    printf("╔════════════════════════════════════════╗\n");
    printf("║   UDP Turn-Taking Chat Client          ║\n");
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
    struct sockaddr_in server_addr;
    socklen_t addr_len = sizeof(server_addr);
    char server_ip[16] = "127.0.0.1";
    int port = PORT;
    
    // Parse command line arguments
    if (argc > 1) {
        strncpy(server_ip, argv[1], sizeof(server_ip) - 1);
    }
    if (argc > 2) {
        port = atoi(argv[2]);
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
    
    // Set receive timeout
    struct timeval tv;
    tv.tv_sec = RECV_TIMEOUT_SEC;
    tv.tv_usec = 0;
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, 
                   &tv, sizeof(tv)) < 0) {
        perror("Setsockopt timeout failed");
    }
    
    // Setup server address structure
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    
    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
        fprintf(stderr, "Invalid server address\n");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    
    print_header();
    printf("Server: %s:%d\n", server_ip, port);
    printf("Commands: /exit to quit\n");
    printf("(Press Ctrl+C to force exit)\n\n");
    printf("╔════════════════════════════════════════╗\n");
    printf("║      Connected! Start chatting         ║\n");
    printf("╚════════════════════════════════════════╝\n\n");
    
    // Main client loop
    while (keep_running) {
        // Send message to server
        printf("Client: ");
        fflush(stdout);
        
        memset(buffer, 0, BUFFER_SIZE);
        if (fgets(buffer, BUFFER_SIZE, stdin) == NULL) {
            if (!keep_running) break;
            continue;
        }
        
        buffer[strcspn(buffer, "\n")] = '\0';
        
        // Skip empty messages
        if (strlen(buffer) == 0) {
            continue;
        }
        
        // Send to server
        if (sendto(sockfd, buffer, strlen(buffer), 0,
                   (const struct sockaddr *)&server_addr, 
                   sizeof(server_addr)) < 0) {
            perror("Sendto failed");
            continue;
        }
        
        // Check if client wants to exit
        if (strcmp(buffer, "/exit") == 0) {
            printf("[%s] Disconnecting...\n", get_timestamp());
            
            // Wait for server acknowledgment
            memset(buffer, 0, BUFFER_SIZE);
            ssize_t n = recvfrom(sockfd, buffer, BUFFER_SIZE - 1, 0,
                                (struct sockaddr *)&server_addr, &addr_len);
            if (n > 0) {
                buffer[n] = '\0';
                printf("%s\n", buffer);
            }
            break;
        }
        
        // Receive response from server
        memset(buffer, 0, BUFFER_SIZE);
        ssize_t n = recvfrom(sockfd, buffer, BUFFER_SIZE - 1, 0,
                            (struct sockaddr *)&server_addr, &addr_len);
        
        if (n < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                printf("[%s] No response from server (timeout).\n", 
                       get_timestamp());
                printf("Server may be unavailable. Continue? (y/n): ");
                
                char choice;
                scanf(" %c", &choice);
                getchar(); // consume newline
                
                if (choice != 'y' && choice != 'Y') {
                    break;
                }
                continue;
            }
            if (errno == EINTR) {
                continue;
            }
            perror("Recvfrom failed");
            continue;
        }
        
        if (n > 0) {
            buffer[n] = '\0';
            
            // Check if server is closing
            if (strstr(buffer, "closing") || strstr(buffer, "Goodbye")) {
                printf("[%s] Server: %s\n", get_timestamp(), buffer);
                printf("[%s] Server closed the connection.\n", get_timestamp());
                break;
            }
            
            printf("[%s] Server: %s\n", get_timestamp(), buffer);
        }
    }
    
    // Cleanup
    close(sockfd);
    printf("[%s] Disconnected. Goodbye!\n", get_timestamp());
    
    return 0;
}
