#include <stdio.h>          /* for printf() and fprintf() */
#include <sys/socket.h>     /* for socket(), bind(), sendto() and recvfrom() */
#include <arpa/inet.h>      /* for sockaddr_in and inet_ntoa() */
#include <stdlib.h>         /* for atoi() and exit() */
#include <string.h>         /* for memset() and strlen() */
#include <unistd.h>         /* for close() */

void DieWithError(char *errorMessage);      /* Error handling function */

#define ECHOMAX 255         /* Longest string to echo */

/**
 * Main function for UDP Broadcast Server
 * Receives messages and broadcasts them to all clients on the network
 */
int main(int argc, char *argv[]) {
    int servSock;                           /* Socket descriptor for server */
    struct sockaddr_in echoServAddr;        /* Local address */
    struct sockaddr_in echoClntAddr;        /* Client address */
    struct sockaddr_in broadcastAddr;       /* Broadcast address */
    unsigned short echoServPort;            /* Server port */
    unsigned int cliAddrLen;                /* Length of incoming message */
    char echoBuffer[ECHOMAX];               /* Buffer for echo string */
    char broadcastIP[] = "255.255.255.255"; /* Local broadcast address */
    int recvMsgSize;                        /* Size of received message */
    int broadcastPermission = 1;            /* Socket opt to permit broadcast */
    
    /* Test for correct number of arguments */
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <UDP SERVER PORT>\n", argv[0]);
        exit(1);
    }
    
    echoServPort = atoi(argv[1]);           /* First arg: local port */
    
    /* Create socket for sending/receiving datagrams */
    if ((servSock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
        DieWithError("socket() failed");
    
    /* Set socket to allow broadcast */
    if (setsockopt(servSock, SOL_SOCKET, SO_BROADCAST, (void *) &broadcastPermission,
                   sizeof(broadcastPermission)) < 0)
        DieWithError("setsockopt() failed");
    
    /* Construct local address structure */
    memset(&echoServAddr, 0, sizeof(echoServAddr));     /* Zero out structure */
    echoServAddr.sin_family = AF_INET;                  /* Internet address family */
    echoServAddr.sin_addr.s_addr = htonl(INADDR_ANY);   /* Any incoming interface */
    echoServAddr.sin_port = htons(echoServPort);        /* Local port */
    
    /* Bind to the local address */
    if (bind(servSock, (struct sockaddr *) &echoServAddr, sizeof(echoServAddr)) < 0)
        DieWithError("bind() failed");
    
    /* Construct broadcast address structure */
    memset(&broadcastAddr, 0, sizeof(broadcastAddr));   /* Zero out structure */
    broadcastAddr.sin_family = AF_INET;                 /* Internet address family */
    broadcastAddr.sin_addr.s_addr = inet_addr(broadcastIP); /* Broadcast IP address */
    broadcastAddr.sin_port = htons(echoServPort);       /* Broadcast port */
    
    printf("UDP Broadcast Server started on port %d\n", echoServPort);
    printf("Broadcasting to address: %s\n", broadcastIP);
    printf("Waiting for client messages to broadcast...\n");
    
    /* Run forever */
    for (;;) {
        /* Set the size of the in-out parameter */
        cliAddrLen = sizeof(echoClntAddr);
        
        /* Block until receive message from a client */
        if ((recvMsgSize = recvfrom(servSock, echoBuffer, ECHOMAX, 0,
                                  (struct sockaddr *) &echoClntAddr, &cliAddrLen)) < 0)
            DieWithError("recvfrom() failed");
        
        printf("Received message from client %s: ", inet_ntoa(echoClntAddr.sin_addr));
        echoBuffer[recvMsgSize] = '\0';     /* Null terminate received string */
        printf("%s\n", echoBuffer);
        
        /* Broadcast received message to all clients */
        if (sendto(servSock, echoBuffer, recvMsgSize, 0,
                   (struct sockaddr *) &broadcastAddr, sizeof(broadcastAddr)) != recvMsgSize)
            DieWithError("sendto() failed");
        
        printf("Message broadcasted to all clients\n");
    }
    /* NOT REACHED */
}
