/* tcp_client.c */
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/select.h>
#include <pthread.h>
#include <errno.h>
#include <signal.h>

#define RCVBUFSIZE 32
#define DEFPORT 12345

void DieWithError(const char *msg){perror(msg);exit(1);}

struct RecvArg{ int sock; };

void *RecvThread(void *a){
    struct RecvArg *r=(struct RecvArg*)a;
    char buf[RCVBUFSIZE];
    ssize_t n;
    while(1){
        n = recv(r->sock, buf, RCVBUFSIZE-1, 0);
        if(n>0){
            buf[n]=0;
            printf("<< %s\n", buf);
            fflush(stdout);
        }else if(n==0){
            fprintf(stderr,"Server closed connection\n");
            break;
        }else{
            if(errno==EINTR) continue;
            perror("recv() failed");
            break;
        }
    }
    return NULL;
}

int main(int argc,char *argv[]){
    int sock;
    struct sockaddr_in servAddr;
    unsigned short port=DEFPORT;
    char *servIP;
    char buf[RCVBUFSIZE];

    if(argc<2||argc>3){
        fprintf(stderr,"Usage: %s <Server IP> [<Port>]\n",argv[0]);
        exit(1);
    }
    servIP=argv[1];
    if(argc==3) port=atoi(argv[2]);

    /* avoid SIGPIPE killing the process if server drops connection */
    signal(SIGPIPE, SIG_IGN);

    if((sock=socket(PF_INET,SOCK_STREAM,IPPROTO_TCP))<0)
        DieWithError("socket() failed");

    memset(&servAddr,0,sizeof(servAddr));
    servAddr.sin_family=AF_INET;
    servAddr.sin_addr.s_addr=inet_addr(servIP);
    servAddr.sin_port=htons(port);

    if(connect(sock,(struct sockaddr*)&servAddr,sizeof(servAddr))<0)
        DieWithError("connect() failed");

    /* --- start recv thread --- */
    pthread_t tid;
    struct RecvArg rarg;
    rarg.sock = sock;
    if(pthread_create(&tid, NULL, RecvThread, &rarg) != 0)
        DieWithError("pthread_create() failed");

    /* Main thread: read stdin and send until user quits */
    while(fgets(buf, RCVBUFSIZE, stdin) != NULL){
        size_t len = strlen(buf);
        if(len > 0 && buf[len-1] == '\n'){ buf[len-1] = '\0'; --len; }

        if(len == 0 || strcmp(buf, "quit") == 0 || strcmp(buf, "exit") == 0){
            close(sock);
            break;
        }

        if(send(sock, buf, len, 0) != (ssize_t)len)
            DieWithError("send() failed");
    }

    /* wait for recv thread to finish */
    pthread_join(tid, NULL);

    return 0;
}
