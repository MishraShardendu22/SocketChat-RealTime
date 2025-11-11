/* tcp_server.c */
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <sys/wait.h>

#define MAXPENDING 5
#define RCVBUFSIZE 32
#define MAXCLIENTS 100
#define DEFPORT 12345

void DieWithError(const char *msg){perror(msg);exit(1);}

/* simple client list */
static int clients[MAXCLIENTS];
static int nclients=0;
static pthread_mutex_t clients_mtx = PTHREAD_MUTEX_INITIALIZER;

void add_client(int s){
    pthread_mutex_lock(&clients_mtx);
    if(nclients<MAXCLIENTS) clients[nclients++]=s;
    pthread_mutex_unlock(&clients_mtx);
}
void remove_client(int s){
    pthread_mutex_lock(&clients_mtx);
    for(int i=0;i<nclients;i++){
        if(clients[i]==s){
            for(int j=i;j<nclients-1;j++) clients[j]=clients[j+1];
            nclients--; break;
        }
    }
    pthread_mutex_unlock(&clients_mtx);
}

/* thread that reads server stdin and broadcasts lines to all clients */
void *ServerSendThread(void *a){
    char buf[RCVBUFSIZE];
    while(fgets(buf,RCVBUFSIZE,stdin)!=NULL){
        size_t len=strlen(buf);
        if(len>0 && buf[len-1]=='\n'){ buf[len-1]=0; --len; }
        if(len==0) continue;
        pthread_mutex_lock(&clients_mtx);
        for(int i=0;i<nclients;i++){
            ssize_t s=send(clients[i],buf,len,0);
            (void)s;
        }
        pthread_mutex_unlock(&clients_mtx);
    }
    return NULL;
}

void HandleTCPClient(int clntSocket){
    char buf[RCVBUFSIZE];
    int recvSize;
    if((recvSize=recv(clntSocket,buf,RCVBUFSIZE-1,0))<0)
        DieWithError("recv() failed");
    while(recvSize>0){
        buf[recvSize]='\0';
        printf("Client: %s\n",buf);
        /* Two-way: echo back to client */
        if(send(clntSocket,buf,recvSize,0)!=recvSize)
            DieWithError("send() failed");
        if((recvSize=recv(clntSocket,buf,RCVBUFSIZE-1,0))<0)
            DieWithError("recv() failed");
    }
    remove_client(clntSocket);
    close(clntSocket);
    printf("Client disconnected\n");
}

/* Thread wrapper */
struct ThreadArgs{int clntSock;};
void* ThreadMain(void *args){
    int clntSock=((struct ThreadArgs*)args)->clntSock;
    free(args);
    pthread_detach(pthread_self());
    HandleTCPClient(clntSock);
    return NULL;
}

int main(int argc,char *argv[]){
    int servSock,clntSock;
    struct sockaddr_in servAddr,clntAddr;
    unsigned short port=DEFPORT;
    unsigned int clntLen;

    if(argc>2){fprintf(stderr,"Usage: %s [<Port>]\n",argv[0]);exit(1);}
    if(argc==2) port=atoi(argv[1]);

    if((servSock=socket(PF_INET,SOCK_STREAM,IPPROTO_TCP))<0)
        DieWithError("socket() failed");

    memset(&servAddr,0,sizeof(servAddr));
    servAddr.sin_family=AF_INET;
    servAddr.sin_addr.s_addr=htonl(INADDR_ANY);
    servAddr.sin_port=htons(port);

    if(bind(servSock,(struct sockaddr*)&servAddr,sizeof(servAddr))<0)
        DieWithError("bind() failed");

    if(listen(servSock,MAXPENDING)<0)
        DieWithError("listen() failed");

    printf("TCP Server running on port %d\n",port);

    /* start server stdin thread for broadcasting to clients */
    pthread_t sendtid;
    if(pthread_create(&sendtid,NULL,ServerSendThread,NULL)!=0)
        DieWithError("pthread_create() failed");

    for(;;){
        clntLen=sizeof(clntAddr);
        if((clntSock=accept(servSock,(struct sockaddr*)&clntAddr,&clntLen))<0)
            DieWithError("accept() failed");
        printf("Connected client: %s:%d\n", inet_ntoa(clntAddr.sin_addr), ntohs(clntAddr.sin_port));
        add_client(clntSock);
        struct ThreadArgs *targs=malloc(sizeof(struct ThreadArgs));
        targs->clntSock=clntSock;
        pthread_t tid;
        if(pthread_create(&tid,NULL,ThreadMain,(void*)targs)!=0)
            DieWithError("pthread_create() failed");
    }
}
