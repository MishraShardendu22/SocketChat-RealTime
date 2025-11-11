#include <stdio.h>          /* for printf() and fprintf() */
#include <sys/socket.h>     /* for socket(), bind(), recvfrom() */
#include <arpa/inet.h>      /* for sockaddr_in and inet_ntoa() */
#include <stdlib.h>         /* for atoi() and exit() */
#include <string.h>         /* for memset() */
#include <unistd.h>         /* for close() */

void DieWithError(char *errorMessage);      /* Error handling function */

#define ECHOMAX 255         /* Longest string to echo */

/**
 * Main function for UDP Broadcast Receiver
 * Listens for broadcast messages on the network
 */
int main(int argc, char *argv[]) {
    int sock;                           /* Socket descriptor */
    struct sockaddr_in recvAddr;        /* Local address */
    struct sockaddr_in senderAddr;      /* Sender address */
    unsigned short recvPort;            /* Port to listen on */
    unsigned int senderLen;             /* Length of sender address */
    char recvBuffer[ECHOMAX];           /* Buffer for received messages */
    int recvMsgSize;                    /* Size of received message */
    
    /* Test for correct number of arguments */
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <UDP PORT>\n", argv[0]);
        exit(1);
    }
    
    recvPort = atoi(argv[1]);           /* First arg: port number */
    
    /* Create socket for receiving datagrams */
    if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
        DieWithError("socket() failed");
    
    /* Construct local address structure */
    memset(&recvAddr, 0, sizeof(recvAddr));       /* Zero out structure */
    recvAddr.sin_family = AF_INET;                /* Internet address family */
    recvAddr.sin_addr.s_addr = htonl(INADDR_ANY); /* Any incoming interface */
    recvAddr.sin_port = htons(recvPort);          /* Local port */
    
    /* Bind to the local address */
    if (bind(sock, (struct sockaddr *) &recvAddr, sizeof(recvAddr)) < 0)
        DieWithError("bind() failed");
    
    printf("UDP Broadcast Receiver listening on port %d\n", recvPort);
    
    /* Run forever */
    for (;;) {
        senderLen = sizeof(senderAddr);
        
        /* Block until receive message from a sender */
        if ((recvMsgSize = recvfrom(sock, recvBuffer, ECHOMAX, 0,
                                  (struct sockaddr *) &senderAddr, &senderLen)) < 0)
            DieWithError("recvfrom() failed");
        
        recvBuffer[recvMsgSize] = '\0'; /* Null terminate string */
        printf("Received broadcast from %s: %s\n",
               inet_ntoa(senderAddr.sin_addr), recvBuffer);
    }
    
    /* NOT REACHED */
}
