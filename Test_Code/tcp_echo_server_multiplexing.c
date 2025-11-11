#include <stdio.h>          /* for printf() and fprintf() */
#include <sys/socket.h>     /* for socket(), bind(), connect(), recv() and send() */
#include <arpa/inet.h>      /* for sockaddr_in and inet_ntoa() */
#include <stdlib.h>         /* for atoi(), exit(), and malloc() */
#include <string.h>         /* for memset() */
#include <unistd.h>         /* for close() and STDIN_FILENO */
#include <sys/time.h>       /* for struct timeval {}

#define MAXPENDING 5        /* Maximum outstanding connection requests */

void DieWithError(char *errorMessage);      /* Error handling function */
void HandleTCPClient(int clntSocket);       /* TCP client handling function */
int CreateTCPServerSocket(unsigned short port);    /* Create TCP server socket */
int AcceptTCPConnection(int servSock);      /* Accept TCP connection */

/**
 * Main function for Multiplexing TCP Echo Server
 * Handles multiple ports using select() for I/O multiplexing
 */
int main(int argc, char *argv[]) {
    int *servSock;                          /* Socket descriptors for server */
    int maxDescriptor;                      /* Maximum socket descriptor value */
    fd_set sockSet;                         /* Set of socket descriptors for select() */
    long timeout;                           /* Timeout value given on command-line */
    struct timeval selTimeout;              /* Timeout for select() */
    int running = 1;                        /* 1 if server should continue running; 0 otherwise */
    int noPorts;                            /* Number of ports specified on command-line */
    int port;                               /* Looping variable for ports */
    unsigned short portNo;                  /* Actual port number */
    
    /* Test for correct number of arguments */
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <Timeout (secs.)> <Port 1> <Port 2> ... <Port N>\n", argv[0]);
        exit(1);
    }
    
    timeout = atol(argv[1]);                /* First arg: Timeout in seconds */
    noPorts = argc - 2;                     /* Number of ports is argument count minus 2 */
    
    /* Allocate list of sockets for incoming connections */
    servSock = (int *) malloc(noPorts * sizeof(int));
    if (servSock == NULL)
        DieWithError("malloc() failed");
    
    maxDescriptor = -1;                     /* Initialize maxDescriptor for use by select() */
    
    /* Create list of ports and sockets to handle ports */
    for (port = 0; port < noPorts; port++) {
        /* Add port to port list. Skip first two arguments */
        portNo = atoi(argv[port + 2]);
        
        /* Create port socket */
        servSock[port] = CreateTCPServerSocket(portNo);
        
        /* Determine if new descriptor is the largest */
        if (servSock[port] > maxDescriptor)
            maxDescriptor = servSock[port];
        
        printf("Server socket created for port %d\n", portNo);
    }
    
    printf("Multiplexing TCP Echo Server started\n");
    printf("Monitoring %d ports with %ld second timeout\n", noPorts, timeout);
    printf("Hit return to shutdown server\n");
    
    while (running) {
        /* Zero socket descriptor vector and set for server sockets */
        /* This must be reset every time select() is called */
        FD_ZERO(&sockSet);
        FD_SET(STDIN_FILENO, &sockSet);     /* Add keyboard to descriptor vector */
        
        for (port = 0; port < noPorts; port++)
            FD_SET(servSock[port], &sockSet);
        
        /* Timeout specification */
        /* This must be reset every time select() is called */
        selTimeout.tv_sec = timeout;        /* timeout (secs.) */
        selTimeout.tv_usec = 0;             /* 0 microseconds */
        
        /* Suspend program until descriptor is ready or timeout */
        if (select(maxDescriptor + 1, &sockSet, NULL, NULL, &selTimeout) == 0) {
            printf("No echo requests for %ld secs...Server still alive\n", timeout);
        } else {
            /* Check for keyboard input (shutdown signal) */
            if (FD_ISSET(STDIN_FILENO, &sockSet)) {
                printf("Shutting down server\n");
                getchar();                  /* Consume the input */
                running = 0;
            }
            
            /* Check each port for incoming connections */
            for (port = 0; port < noPorts; port++) {
                if (FD_ISSET(servSock[port], &sockSet)) {
                    printf("Request on port %d: ", atoi(argv[port + 2]));
                    HandleTCPClient(AcceptTCPConnection(servSock[port]));
                }
            }
        }
    }
    
    /* Close all server sockets */
    for (port = 0; port < noPorts; port++)
        close(servSock[port]);
    
    free(servSock);                         /* Free list of sockets */
    exit(0);
}

/**
 * CreateTCPServerSocket - Create TCP server socket
 * @port: Port number for the server
 * Returns: Socket descriptor for server
 */
int CreateTCPServerSocket(unsigned short port) {
    int sock;                               /* Socket to create */
    struct sockaddr_in echoServAddr;        /* Local address */
    
    /* Create socket for incoming connections */
    if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        DieWithError("socket() failed");
    
    /* Construct local address structure */
    memset(&echoServAddr, 0, sizeof(echoServAddr));     /* Zero out structure */
    echoServAddr.sin_family = AF_INET;                  /* Internet address family */
    echoServAddr.sin_addr.s_addr = htonl(INADDR_ANY);   /* Any incoming interface */
    echoServAddr.sin_port = htons(port);                /* Local port */
    
    /* Bind to the local address */
    if (bind(sock, (struct sockaddr *) &echoServAddr, sizeof(echoServAddr)) < 0)
        DieWithError("bind() failed");
    
    /* Mark the socket so it will listen for incoming connections */
    if (listen(sock, MAXPENDING) < 0)
        DieWithError("listen() failed");
    
    return sock;
}

/**
 * AcceptTCPConnection - Accept TCP connection
 * @servSock: Server socket descriptor
 * Returns: Socket descriptor for accepted connection
 */
int AcceptTCPConnection(int servSock) {
    int clntSock;                           /* Socket descriptor for client */
    struct sockaddr_in echoClntAddr;        /* Client address */
    unsigned int clntLen;                   /* Length of client address data structure */
    
    /* Set the size of the in-out parameter */
    clntLen = sizeof(echoClntAddr);
    
    /* Wait for a client to connect */
    if ((clntSock = accept(servSock, (struct sockaddr *) &echoClntAddr, &clntLen)) < 0)
        DieWithError("accept() failed");
    
    /* clntSock is connected to a client! */
    printf("Client %s connected\n", inet_ntoa(echoClntAddr.sin_addr));
    
    return clntSock;
}

#define RCVBUFSIZE 32       /* Size of receive buffer */

/**
 * HandleTCPClient - Handle TCP client communication
 * Receives data from client and echoes it back
 * @clntSocket: Socket descriptor for client connection
 */
void HandleTCPClient(int clntSocket) {
    char echoBuffer[RCVBUFSIZE];            /* Buffer for echo string */
    int recvMsgSize;                        /* Size of received message */
    
    /* Receive message from client */
    if ((recvMsgSize = recv(clntSocket, echoBuffer, RCVBUFSIZE, 0)) < 0)
        DieWithError("recv() failed");
    
    /* Send received string and receive again until end of transmission */
    while (recvMsgSize > 0) {               /* zero indicates end of transmission */
        /* Echo message back to client */
        if (send(clntSocket, echoBuffer, recvMsgSize, 0) != recvMsgSize)
            DieWithError("send() failed");
        
        /* See if there is more data to receive */
        if ((recvMsgSize = recv(clntSocket, echoBuffer, RCVBUFSIZE, 0)) < 0)
            DieWithError("recv() failed");
    }
    
    close(clntSocket);      /* Close client socket */
    printf("Client connection closed\n");
}
