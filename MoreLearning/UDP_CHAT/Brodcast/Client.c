/*
 * UDP Broadcast Chat Client
 * Connects to broadcast server and enables real-time messaging
 * Features: Multi-threaded I/O, graceful disconnection, commands
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#include <signal.h>
#include <errno.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define USERNAME_SIZE 32

// Global variables
int sockfd;
struct sockaddr_in server_addr;
char username[USERNAME_SIZE];
volatile sig_atomic_t keep_running = 1;
pthread_t recv_thread;

// Function prototypes
void signal_handler(int sig);
void cleanup_client();
void *receive_messages(void *arg);
void print_help();

// Signal handler for graceful shutdown
void signal_handler(int sig) {
    printf("\n[CLIENT] Shutting down...\n");
    keep_running = 0;
}

// Print help/commands
void print_help() {
    printf("\n╔════════════════════════════════════════╗\n");
    printf("║            Available Commands          ║\n");
    printf("╠════════════════════════════════════════╣\n");
    printf("║ /help    - Show this help message     ║\n");
    printf("║ /users   - List active users           ║\n");
    printf("║ /clear   - Clear screen                ║\n");
    printf("║ /exit    - Exit the chat               ║\n");
    printf("╚════════════════════════════════════════╝\n\n");
}

// Thread function to receive messages
void *receive_messages(void *arg) {
    char buffer[BUFFER_SIZE];
    socklen_t addr_len = sizeof(server_addr);
    
    while (keep_running) {
        memset(buffer, 0, BUFFER_SIZE);
        
        ssize_t n = recvfrom(sockfd, buffer, BUFFER_SIZE - 1, 0,
                            (struct sockaddr *)&server_addr, &addr_len);
        
        if (n < 0) {
            if (errno == EINTR || !keep_running) {
                break;
            }
            perror("[CLIENT] Receive error");
            continue;
        }
        
        if (n > 0) {
            buffer[n] = '\0';
            printf("%s", buffer);
            fflush(stdout);
        }
    }
    
    return NULL;
}

// Cleanup and disconnect
void cleanup_client() {
    char leave_msg[BUFFER_SIZE];
    snprintf(leave_msg, BUFFER_SIZE, "LEAVE:%s", username);
    
    sendto(sockfd, leave_msg, strlen(leave_msg), 0,
           (const struct sockaddr *)&server_addr, sizeof(server_addr));
    
    keep_running = 0;
    
    if (recv_thread) {
        pthread_cancel(recv_thread);
        pthread_join(recv_thread, NULL);
    }
    
    if (sockfd >= 0) {
        close(sockfd);
    }
    
    printf("[CLIENT] Disconnected. Goodbye!\n");
}

int main(int argc, char *argv[]) {
    char buffer[BUFFER_SIZE];
    char message[BUFFER_SIZE];
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
    
    // Setup server address structure
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    
    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
        fprintf(stderr, "Invalid server address\n");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    
    // Display welcome screen
    printf("╔════════════════════════════════════════╗\n");
    printf("║     UDP Broadcast Chat Client          ║\n");
    printf("╚════════════════════════════════════════╝\n");
    printf("Server: %s:%d\n", server_ip, port);
    printf("\nEnter your username: ");
    fflush(stdout);
    
    if (fgets(buffer, USERNAME_SIZE, stdin) == NULL) {
        fprintf(stderr, "Failed to read username\n");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    
    buffer[strcspn(buffer, "\n")] = '\0';
    
    if (strlen(buffer) == 0) {
        fprintf(stderr, "Username cannot be empty\n");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    
    strncpy(username, buffer, USERNAME_SIZE - 1);
    username[USERNAME_SIZE - 1] = '\0';
    
    // Send join message to server
    snprintf(message, BUFFER_SIZE, "JOIN:%s", username);
    if (sendto(sockfd, message, strlen(message), 0,
               (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Failed to send join message");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    
    // Create thread to receive messages
    if (pthread_create(&recv_thread, NULL, receive_messages, NULL) != 0) {
        perror("Thread creation failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    
    printf("\n╔════════════════════════════════════════╗\n");
    printf("║        Connected! Start chatting       ║\n");
    printf("║      Type /help for commands           ║\n");
    printf("╚════════════════════════════════════════╝\n\n");
    
    // Main loop - send messages
    while (keep_running) {
        memset(buffer, 0, BUFFER_SIZE);
        
        if (fgets(buffer, BUFFER_SIZE, stdin) == NULL) {
            if (!keep_running) break;
            continue;
        }
        
        // Remove newline
        buffer[strcspn(buffer, "\n")] = '\0';
        
        // Skip empty messages
        if (strlen(buffer) == 0) {
            continue;
        }
        
        // Handle commands
        if (strcmp(buffer, "/exit") == 0 || strcmp(buffer, "/quit") == 0) {
            break;
        } else if (strcmp(buffer, "/help") == 0) {
            print_help();
            continue;
        } else if (strcmp(buffer, "/clear") == 0) {
            system("clear || cls");
            continue;
        } else if (strcmp(buffer, "/users") == 0) {
            // Send command to server
            sendto(sockfd, buffer, strlen(buffer), 0,
                   (const struct sockaddr *)&server_addr, sizeof(server_addr));
            continue;
        }
        
        // Format and send regular message
        snprintf(message, BUFFER_SIZE, "%s: %s\n", username, buffer);
        
        if (sendto(sockfd, message, strlen(message), 0,
                   (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
            perror("[CLIENT] Send failed");
        }
    }
    
    cleanup_client();
    return 0;
}
