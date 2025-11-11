#include <stdio.h>          /* for printf() and fprintf() */
#include <signal.h>         /* for signal handling functions */
#include <unistd.h>         /* for pause() */
#include <stdlib.h>         /* for exit() */

void DieWithError(char *errorMessage);          /* Error handling function */
void InterruptSignalHandler(int signalType);    /* Signal handler for SIGINT */

/**
 * Main function demonstrating signal handling
 * Sets up a signal handler for SIGINT (Ctrl+C) and waits for signals
 */
int main(int argc, char *argv[]) {
    struct sigaction handler;               /* Signal handler specification structure */
    
    /* Set handler function */
    handler.sa_handler = InterruptSignalHandler;
    
    /* Create mask that masks all signals */
    if (sigfillset(&handler.sa_mask) < 0)
        DieWithError("sigfillset() failed");
    
    handler.sa_flags = 0;
    
    /* Set signal handling for interrupt signals */
    if (sigaction(SIGINT, &handler, 0) < 0)
        DieWithError("sigaction() failed");
    
    printf("Signal handling example started\n");
    printf("Press Ctrl+C to trigger signal handler\n");
    printf("The program will handle the interrupt gracefully\n");
    
    /* Suspend program until signal received */
    for (;;) {
        printf("Waiting for signal... (Press Ctrl+C)\n");
        pause();                            /* Wait for signal */
        printf("Signal handled, continuing...\n");
    }
    
    exit(0);
}

/**
 * InterruptSignalHandler - Handle SIGINT signal
 * @signalType: Type of signal received
 */
void InterruptSignalHandler(int signalType) {
    printf("\nInterrupt signal received (Signal: %d)\n", signalType);
    printf("Performing cleanup before exit...\n");
    printf("Goodbye!\n");
    exit(1);
}
