#include <stdio.h>          /* for printf() and fprintf() */
#include <sys/socket.h>     /* for socket(), connect(), sendto(), and recvfrom() */
#include <arpa/inet.h>      /* for sockaddr_in and inet_addr() */
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
 * Main function for UDP Echo Client
 * Connects to UDP server, sends message, and receives echo
 */
int main(int argc, char *argv[]) {
    int clientSock;                         /* Socket descriptor */
    struct sockaddr_in echoServAddr;        /* Echo server address */
    struct sockaddr_in echoClntAddr;        /* Client address */
    unsigned short echoServPort;            /* Echo server port */
    unsigned short echoClntPort;            /* Echo client port */
    char *servIP;                           /* Server IP address (dotted quad) */
    char *echoString;                       /* String to send to echo server */
    char echoBuffer[ECHOMAX];               /* Buffer for echo string */
    unsigned int echoStringLen;             /* Length of string to echo */
    int recvMsgSize;                        /* Size of received message */
    unsigned int fromSize;                  /* In-out of address size for recvfrom() */
    
    /* Test for correct number of arguments */
    if ((argc < 4) || (argc > 5)) {
        fprintf(stderr, "Usage: %s <Server IP> <Echo Word> <Client Port> [<Echo Port>]\n", argv[0]);
        exit(1);
    }
    
    servIP = argv[1];                       /* First arg: server IP address (dotted quad) */
    echoString = argv[2];                   /* Second arg: string to echo */
    echoClntPort = atoi(argv[3]);          /* Third arg: client port */
    
    /* Fourth arg (optional): server port - use 7 if not specified */
    if (argc == 5)
        echoServPort = atoi(argv[4]);       /* Use given port, if any */
    else
        echoServPort = 7;                   /* 7 is the well-known port for the echo service */
    
    /* Create a datagram/UDP socket */
    if ((clientSock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
        DieWithError("socket() failed");
    
    /* Construct the client address structure */
    memset(&echoClntAddr, 0, sizeof(echoClntAddr));     /* Zero out structure */
    echoClntAddr.sin_family = AF_INET;                  /* Internet address family */
    echoClntAddr.sin_addr.s_addr = htonl(INADDR_ANY);   /* Any incoming interface */
    echoClntAddr.sin_port = htons(echoClntPort);        /* Client port */
    
    /* Bind to the client address */
    if (bind(clientSock, (struct sockaddr *) &echoClntAddr, sizeof(echoClntAddr)) < 0)
        DieWithError("bind() failed");
    
    /* Construct the server address structure */
    memset(&echoServAddr, 0, sizeof(echoServAddr));     /* Zero out structure */
    echoServAddr.sin_family = AF_INET;                  /* Internet address family */
    echoServAddr.sin_addr.s_addr = inet_addr(servIP);   /* Server IP address */
    echoServAddr.sin_port = htons(echoServPort);        /* Server port */
    
    echoStringLen = strlen(echoString);     /* Determine input length */
    
    /* Send the string to the server */
    if (sendto(clientSock, echoString, echoStringLen, 0,
               (struct sockaddr *) &echoServAddr, sizeof(echoServAddr)) != echoStringLen)
        DieWithError("sendto() sent a different number of bytes than expected");
    
    /* Recv a response */
    fromSize = sizeof(echoServAddr);
    if ((recvMsgSize = recvfrom(clientSock, echoBuffer, ECHOMAX, 0,
                               (struct sockaddr *) &echoServAddr, &fromSize)) != echoStringLen)
        DieWithError("recvfrom() failed");
    
    echoBuffer[recvMsgSize] = '\0';         /* Terminate the string! */
    printf("Received: %s\n", echoBuffer);   /* Print the echo buffer */
    
    close(clientSock);
    exit(0);
}
