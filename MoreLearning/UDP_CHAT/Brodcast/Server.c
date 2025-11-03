/*
 * UDP Broadcast Chat Server
 * Supports multiple clients with message broadcasting
 * Features: Thread-safe client management, proper error handling
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#include <errno.h>
#include <signal.h>
#include <time.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define MAX_CLIENTS 50
#define USERNAME_SIZE 32

// Client structure
typedef struct {
    struct sockaddr_in address;
    socklen_t addr_len;
    char username[USERNAME_SIZE];
    time_t last_active;
    int is_active;
} Client;

// Global variables
Client clients[MAX_CLIENTS];
int client_count = 0;
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;
int server_sockfd;
volatile sig_atomic_t keep_running = 1;

// Function prototypes
void signal_handler(int sig);
void cleanup_server();
int find_client(struct sockaddr_in *addr);
int add_client(struct sockaddr_in *addr, socklen_t addr_len, const char *username);
void remove_client(int index);
void broadcast_message(const char *message, struct sockaddr_in *sender_addr);
void send_client_list(struct sockaddr_in *addr, socklen_t addr_len);
const char* get_timestamp();

// Signal handler for graceful shutdown
void signal_handler(int sig) {
    printf("\n[SERVER] Received signal %d, shutting down...\n", sig);
    keep_running = 0;
}

// Get formatted timestamp
const char* get_timestamp() {
    static char time_str[20];
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    strftime(time_str, sizeof(time_str), "%H:%M:%S", t);
    return time_str;
}

// Find client by address
int find_client(struct sockaddr_in *addr) {
    for (int i = 0; i < client_count; i++) {
        if (clients[i].is_active &&
            clients[i].address.sin_addr.s_addr == addr->sin_addr.s_addr &&
            clients[i].address.sin_port == addr->sin_port) {
            return i;
        }
    }
    return -1;
}

// Add new client
int add_client(struct sockaddr_in *addr, socklen_t addr_len, const char *username) {
    pthread_mutex_lock(&clients_mutex);
    
    // Check if client already exists
    int existing = find_client(addr);
    if (existing != -1) {
        // Update existing client
        strncpy(clients[existing].username, username, USERNAME_SIZE - 1);
        clients[existing].username[USERNAME_SIZE - 1] = '\0';
        clients[existing].last_active = time(NULL);
        pthread_mutex_unlock(&clients_mutex);
        return existing;
    }
    
    // Add new client
    if (client_count >= MAX_CLIENTS) {
        pthread_mutex_unlock(&clients_mutex);
        return -1;
    }
    
    clients[client_count].address = *addr;
    clients[client_count].addr_len = addr_len;
    strncpy(clients[client_count].username, username, USERNAME_SIZE - 1);
    clients[client_count].username[USERNAME_SIZE - 1] = '\0';
    clients[client_count].last_active = time(NULL);
    clients[client_count].is_active = 1;
    
    int index = client_count;
    client_count++;
    
    printf("[%s] Client '%s' joined (%s:%d) - Total clients: %d\n",
           get_timestamp(),
           clients[index].username,
           inet_ntoa(addr->sin_addr),
           ntohs(addr->sin_port),
           client_count);
    
    pthread_mutex_unlock(&clients_mutex);
    return index;
}

// Remove client
void remove_client(int index) {
    pthread_mutex_lock(&clients_mutex);
    
    if (index >= 0 && index < client_count) {
        printf("[%s] Client '%s' left - Total clients: %d\n",
               get_timestamp(),
               clients[index].username,
               client_count - 1);
        
        // Shift remaining clients
        for (int i = index; i < client_count - 1; i++) {
            clients[i] = clients[i + 1];
        }
        client_count--;
    }
    
    pthread_mutex_unlock(&clients_mutex);
}

// Broadcast message to all clients except sender
void broadcast_message(const char *message, struct sockaddr_in *sender_addr) {
    pthread_mutex_lock(&clients_mutex);
    
    for (int i = 0; i < client_count; i++) {
        if (!clients[i].is_active) continue;
        
        // Don't send back to sender
        if (sender_addr && 
            clients[i].address.sin_addr.s_addr == sender_addr->sin_addr.s_addr &&
            clients[i].address.sin_port == sender_addr->sin_port) {
            continue;
        }
        
        ssize_t sent = sendto(server_sockfd, message, strlen(message), 0,
                              (struct sockaddr *)&clients[i].address, 
                              clients[i].addr_len);
        
        if (sent < 0) {
            perror("[SERVER] Sendto failed");
        }
    }
    
    pthread_mutex_unlock(&clients_mutex);
}

// Send list of active clients
void send_client_list(struct sockaddr_in *addr, socklen_t addr_len) {
    char response[BUFFER_SIZE];
    int offset = 0;
    
    pthread_mutex_lock(&clients_mutex);
    
    offset += snprintf(response + offset, BUFFER_SIZE - offset, 
                      "[SERVER] Active users (%d):\n", client_count);
    
    for (int i = 0; i < client_count && offset < BUFFER_SIZE - 50; i++) {
        if (clients[i].is_active) {
            offset += snprintf(response + offset, BUFFER_SIZE - offset,
                             "  - %s\n", clients[i].username);
        }
    }
    
    pthread_mutex_unlock(&clients_mutex);
    
    sendto(server_sockfd, response, strlen(response), 0,
           (struct sockaddr *)addr, addr_len);
}

// Cleanup and close server
void cleanup_server() {
    char goodbye_msg[] = "[SERVER] Server is shutting down. Goodbye!\n";
    broadcast_message(goodbye_msg, NULL);
    
    if (server_sockfd >= 0) {
        close(server_sockfd);
    }
    
    pthread_mutex_destroy(&clients_mutex);
    printf("[SERVER] Cleanup complete.\n");
}

int main(int argc, char *argv[]) {
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len;
    char buffer[BUFFER_SIZE];
    int port = PORT;
    
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
    if ((server_sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }
    
    // Set socket options
    int optval = 1;
    if (setsockopt(server_sockfd, SOL_SOCKET, SO_REUSEADDR, 
                   &optval, sizeof(optval)) < 0) {
        perror("Setsockopt failed");
    }
    
    // Initialize server address structure
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);
    
    // Bind socket to address
    if (bind(server_sockfd, (const struct sockaddr *)&server_addr, 
             sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(server_sockfd);
        exit(EXIT_FAILURE);
    }
    
    printf("╔════════════════════════════════════════╗\n");
    printf("║   UDP Broadcast Chat Server Started    ║\n");
    printf("╚════════════════════════════════════════╝\n");
    printf("Port: %d\n", port);
    printf("Max Clients: %d\n", MAX_CLIENTS);
    printf("Waiting for clients...\n\n");
    
    // Main server loop
    while (keep_running) {
        memset(buffer, 0, BUFFER_SIZE);
        addr_len = sizeof(client_addr);
        
        // Receive message from client
        ssize_t n = recvfrom(server_sockfd, buffer, BUFFER_SIZE - 1, 0,
                            (struct sockaddr *)&client_addr, &addr_len);
        
        if (n < 0) {
            if (errno == EINTR) {
                continue; // Interrupted by signal
            }
            perror("Recvfrom failed");
            continue;
        }
        
        if (n == 0) continue;
        
        buffer[n] = '\0';
        
        // Parse message type
        if (strncmp(buffer, "JOIN:", 5) == 0) {
            // Join message
            char username[USERNAME_SIZE];
            sscanf(buffer + 5, "%31s", username);
            
            int client_idx = add_client(&client_addr, addr_len, username);
            
            if (client_idx == -1) {
                char error_msg[] = "[SERVER] Server is full. Try again later.\n";
                sendto(server_sockfd, error_msg, strlen(error_msg), 0,
                       (struct sockaddr *)&client_addr, addr_len);
            } else {
                // Send welcome message to new client
                char welcome[BUFFER_SIZE];
                snprintf(welcome, BUFFER_SIZE, 
                        "[SERVER] Welcome %s! Type /users to see active users.\n",
                        username);
                sendto(server_sockfd, welcome, strlen(welcome), 0,
                       (struct sockaddr *)&client_addr, addr_len);
                
                // Notify all clients
                char join_msg[BUFFER_SIZE];
                snprintf(join_msg, BUFFER_SIZE, 
                        "[%s] %s joined the chat\n", get_timestamp(), username);
                broadcast_message(join_msg, &client_addr);
            }
            
        } else if (strncmp(buffer, "LEAVE:", 6) == 0) {
            // Leave message
            int client_idx = find_client(&client_addr);
            if (client_idx != -1) {
                char leave_msg[BUFFER_SIZE];
                snprintf(leave_msg, BUFFER_SIZE, 
                        "[%s] %s left the chat\n", 
                        get_timestamp(), clients[client_idx].username);
                remove_client(client_idx);
                broadcast_message(leave_msg, NULL);
            }
            
        } else if (strcmp(buffer, "/users") == 0) {
            // Send user list
            send_client_list(&client_addr, addr_len);
            
        } else {
            // Regular message - broadcast to all
            int client_idx = find_client(&client_addr);
            if (client_idx != -1) {
                clients[client_idx].last_active = time(NULL);
                
                char formatted_msg[BUFFER_SIZE];
                snprintf(formatted_msg, BUFFER_SIZE, "[%s] %s", 
                        get_timestamp(), buffer);
                
                printf("%s", formatted_msg);
                broadcast_message(formatted_msg, &client_addr);
            }
        }
    }
    
    cleanup_server();
    return 0;
}
