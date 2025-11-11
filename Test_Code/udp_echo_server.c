#include <stdio.h>          /* for printf() and fprintf() */
#include <sys/socket.h>     /* for socket(), bind(), sendto() and recvfrom() */
#include <arpa/inet.h>      /* for sockaddr_in and inet_ntoa() */
#include <stdlib.h>         /* for atoi() and exit() */
#include <string.h>         /* for memset() */
#include <unistd.h>         /* for close() */
#include <errno.h>          /* for errno */

void DieWithError(const char * msg) {       /* Error handling function */
    perror(msg);
    exit(1);
}
#define ECHOMAX 255         /* Longest string to echo */

/**
 * Main function for UDP Echo Server
 * Creates a UDP server socket and echoes received messages back to clients
 */
int main(int argc, char *argv[]) {
    int servSock;                           /* Socket descriptor for server */
    struct sockaddr_in echoServAddr;        /* Local address */
    struct sockaddr_in echoClntAddr;        /* Client address */
    unsigned short echoServPort;            /* Server port */
    unsigned int cliAddrLen;                /* Length of incoming message */
    char echoBuffer[ECHOMAX];               /* Buffer for echo string */
    int recvMsgSize;                        /* Size of received message */
    
    /* Test for correct number of arguments */
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <UDP SERVER PORT>\n", argv[0]);
        exit(1);
    }
    
    echoServPort = atoi(argv[1]);           /* First arg: local port */
    
    /* Create socket for sending/receiving datagrams */
    if ((servSock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
        DieWithError("socket() failed");
    
    /* Construct local address structure */
    memset(&echoServAddr, 0, sizeof(echoServAddr));     /* Zero out structure */
    echoServAddr.sin_family = AF_INET;                  /* Internet address family */
    echoServAddr.sin_addr.s_addr = htonl(INADDR_ANY);   /* Any incoming interface */
    echoServAddr.sin_port = htons(echoServPort);        /* Local port */
    
    /* Bind to the local address */
    if (bind(servSock, (struct sockaddr *) &echoServAddr, sizeof(echoServAddr)) < 0)
        DieWithError("bind() failed");
    
    printf("UDP Echo Server started on port %d\n", echoServPort);
    printf("Waiting for client connections...\n");
    
    /* Run forever */
    for (;;) {
        /* Set the size of the in-out parameter */
        cliAddrLen = sizeof(echoClntAddr);
        
        /* Block until receive message from a client */
        if ((recvMsgSize = recvfrom(servSock, echoBuffer, ECHOMAX, 0,
                                  (struct sockaddr *) &echoClntAddr, &cliAddrLen)) < 0)
            DieWithError("recvfrom() failed");
        
        echoBuffer[recvMsgSize] = '\0';  // Null terminate for safe printing

        printf("Handling client %s | Received: %s\n",
               inet_ntoa(echoClntAddr.sin_addr), echoBuffer);
        
        /* Send received datagram back to the client */
        if (sendto(servSock, echoBuffer, recvMsgSize, 0,
                   (struct sockaddr *) &echoClntAddr, sizeof(echoClntAddr)) != recvMsgSize)
            DieWithError("sendto() sent a different number of bytes than expected");
        
        printf("Message echoed back to client\n");
    }
    /* NOT REACHED */
}
