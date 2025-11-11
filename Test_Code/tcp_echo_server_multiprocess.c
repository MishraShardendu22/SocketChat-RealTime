#include <stdio.h>          /* for printf() and fprintf() */
#include <sys/socket.h>     /* for socket(), bind(), connect(), recv() and send() */
#include <arpa/inet.h>      /* for sockaddr_in and inet_ntoa() */
#include <stdlib.h>         /* for atoi() and exit() */
#include <string.h>         /* for memset() */
#include <unistd.h>         /* for close() and fork() */
#include <sys/wait.h>       /* for waitpid() */

#define MAXPENDING 5        /* Maximum outstanding connection requests */

void DieWithError(char *errorMessage); 		/* Error handling function */
void HandleTCPClient(int clntSocket); 		/* TCP client handling function */
int CreateTCPServerSocket(unsigned short port); 	    /* Create TCP server socket */
int AcceptTCPConnection(int servSock); 		/* Accept TCP connection */

/**
 * Main function for Multi-Process TCP Echo Server
 * Creates child processes to handle each client connection
 */
int main(int argc, char *argv[]) {
    int servSock; 					/* Socket descriptor for server */
    int clntSock; 					/* Socket descriptor for client */
    unsigned short echoServPort; 				/* Server port */
    pid_t processID; 					/* Process ID from fork() */
    unsigned int childProcCount = 0; 			/* Number of child processes */
    
    /* Test for correct number of arguments */
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <Server Port>\n", argv[0]);
        exit(1);
    }
    
    echoServPort = atoi(argv[1]); 			/* First arg: local port */
    
    /* Create server socket */
    servSock = CreateTCPServerSocket(echoServPort);
    
    printf("Multi-Process TCP Echo Server started on port %d\n", echoServPort);
    
    /* Run forever */
    for (;;) {
        /* Accept new client connection */
        clntSock = AcceptTCPConnection(servSock);
        
        /* Fork child process to handle client */
        if ((processID = fork()) < 0)
            DieWithError("fork() failed");
        else if (processID == 0) {			/* This is the child process */
            close(servSock); 				/* Child closes listening socket */
            HandleTCPClient(clntSock); 		/* Process client request */
            exit(0); 					/* Child process terminates */
        }
        
        printf("Created child process with PID %d\n", processID);
        close(clntSock); 				/* Parent closes child socket */
        childProcCount++; 				/* Increment number of outstanding child processes */
        
        /* Clean up zombies */
        while (childProcCount) {
            processID = waitpid((pid_t) -1, NULL, WNOHANG); 	/* Non-blocking wait */
            if (processID < 0) 			/* waitpid() error? */
                DieWithError("waitpid() failed");
            else if (processID == 0)			/* No zombie to wait for */
                break;
            else
                childProcCount--; 			/* Cleaned up after a child */
        }
    }
    /* NOT REACHED */
}

/**
 * CreateTCPServerSocket - Create TCP server socket
 * @port: Port number for the server
 * Returns: Socket descriptor for server
 */
int CreateTCPServerSocket(unsigned short port) {
    int sock; 				/* Socket to create */
    struct sockaddr_in echoServAddr; 		/* Local address */
    
    /* Create socket for incoming connections */
    if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        DieWithError("socket() failed");
    
    /* Construct local address structure */
    memset(&echoServAddr, 0, sizeof(echoServAddr)); 	/* Zero out structure */
    echoServAddr.sin_family = AF_INET; 				/* Internet address family */
    echoServAddr.sin_addr.s_addr = htonl(INADDR_ANY); 	   /* Any incoming interface */
    echoServAddr.sin_port = htons(port); 				/* Local port */
    
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
    int clntSock; 			/* Socket descriptor for client */
    struct sockaddr_in echoClntAddr; 		/* Client address */
    unsigned int clntLen; 			/* Length of client address data structure */
    
    /* Set the size of the in-out parameter */
    clntLen = sizeof(echoClntAddr);
    
    /* Wait for a client to connect */
    if ((clntSock = accept(servSock, (struct sockaddr *) &echoClntAddr, &clntLen)) < 0)
        DieWithError("accept() failed");
    
    /* clntSock is connected to a client! */
    printf("Handling client %s\n", inet_ntoa(echoClntAddr.sin_addr));
    
    return clntSock;
}

#define RCVBUFSIZE 32 			/* Size of receive buffer */

/**
 * HandleTCPClient - Handle TCP client communication
 * Receives data from client and echoes it back
 * @clntSocket: Socket descriptor for client connection
 */
void HandleTCPClient(int clntSocket) {
    char echoBuffer[RCVBUFSIZE]; 			/* Buffer for echo string */
    int recvMsgSize; 				/* Size of received message */
    
    /* Receive message from client */
    if ((recvMsgSize = recv(clntSocket, echoBuffer, RCVBUFSIZE, 0)) < 0)
        DieWithError("recv() failed");
    
    /* Send received string and receive again until end of transmission */
    while (recvMsgSize > 0) {			/* zero indicates end of transmission */
        /* Echo message back to client */
        if (send(clntSocket, echoBuffer, recvMsgSize, 0) != recvMsgSize)
            DieWithError("send() failed");
        
        /* See if there is more data to receive */
        if ((recvMsgSize = recv(clntSocket, echoBuffer, RCVBUFSIZE, 0)) < 0)
            DieWithError("recv() failed");
    }
    
    close(clntSocket); 			/* Close client socket */
    printf("Child process: Client connection closed\n");
}
