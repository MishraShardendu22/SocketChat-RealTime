#include <stdio.h>          /* for printf() and fprintf() */
#include <sys/socket.h>     /* for socket(), bind(), connect(), recv() and send() */
#include <arpa/inet.h>      /* for sockaddr_in and inet_ntoa() */
#include <stdlib.h>         /* for atoi() and exit() */
#include <string.h>         /* for memset() */
#include <unistd.h>         /* for close(), fork(), and getpid() */
#include <sys/wait.h>       /* for waitpid() */

#define MAXPENDING 5        /* Maximum outstanding connection requests */

void DieWithError(char *errorMessage);      /* Error handling function */
void ProcessMain(int servSock);             /* Main program of process */
void HandleTCPClient(int clntSocket);       /* TCP client handling function */
int CreateTCPServerSocket(unsigned short port);    /* Create TCP server socket */
int AcceptTCPConnection(int servSock);      /* Accept TCP connection */

/**
 * Main function for Constrained Multi-Process TCP Echo Server
 * Creates a limited number of processes to handle client connections
 */
int main(int argc, char *argv[]) {
    int servSock;                           /* Socket descriptor for server */
    unsigned short echoServPort;            /* Server port */
    pid_t processID;                        /* Process ID */
    unsigned int processLimit;              /* Number of child processes to create */
    unsigned int processCt;                 /* Process counter */
    
    /* Test for correct number of arguments */
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <SERVER PORT> <FORK LIMIT>\n", argv[0]);
        exit(1);
    }
    
    echoServPort = atoi(argv[1]);           /* First arg: local port */
    processLimit = atoi(argv[2]);           /* Second arg: number of child processes */
    
    /* Create server socket */
    servSock = CreateTCPServerSocket(echoServPort);
    
    printf("Constrained Multi-Process TCP Echo Server started on port %d\n", echoServPort);
    printf("Creating %d child processes...\n", processLimit);
    
    /* Create specified number of child processes */
    for (processCt = 0; processCt < processLimit; processCt++) {
        /* Fork child process */
        if ((processID = fork()) < 0)
            DieWithError("fork() failed");
        else if (processID == 0) {          /* If this is the child process */
            ProcessMain(servSock);          /* Child handles clients */
            exit(0);                        /* Child should never reach here */
        }
        printf("Created child process %d with PID %d\n", processCt + 1, processID);
    }
    
    /* Parent process waits for children to complete (they run forever) */
    printf("Parent process waiting for children to complete...\n");
    for (processCt = 0; processCt < processLimit; processCt++) {
        wait(NULL);                         /* Wait for child to terminate */
    }
    
    close(servSock);
    exit(0);                                /* The children will carry on */
}

/**
 * ProcessMain - Main function for child process
 * Each child process accepts connections and handles clients
 * @servSock: Server socket descriptor
 */
void ProcessMain(int servSock) {
    int clntSock;                           /* Socket descriptor for client connection */
    
    printf("Child process %d started and waiting for connections...\n", 
           (unsigned int) getpid());
    
    /* Run forever - each child handles clients sequentially */
    for (;;) {
        /* Accept connection (blocks until client connects) */
        clntSock = AcceptTCPConnection(servSock);
        
        printf("Child process %d handling client\n", (unsigned int) getpid());
        
        /* Handle the client */
        HandleTCPClient(clntSock);
    }
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
    printf("Handling client %s\n", inet_ntoa(echoClntAddr.sin_addr));
    
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
    printf("Process %d: Client connection closed\n", (unsigned int) getpid());
}
