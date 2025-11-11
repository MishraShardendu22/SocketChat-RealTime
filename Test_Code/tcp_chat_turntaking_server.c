// tcp_turn_server.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define P 12345
#define B 512

int main(int argc,char **argv){
    unsigned short port=P;
    int server_first=0;           
    if(argc>=2) port=(unsigned short)atoi(argv[1]);
    if(argc>=3) server_first=(argv[2][0]=='s'||argv[2][0]=='S');

    int s=socket(AF_INET,SOCK_STREAM,0); if(s<0){perror("socket");return 1;}
    int on=1; setsockopt(s,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on));
    struct sockaddr_in a={.sin_family=AF_INET,.sin_addr.s_addr=htonl(INADDR_ANY),.sin_port=htons(port)};
    if(bind(s,(struct sockaddr*)&a,sizeof(a))<0){perror("bind");close(s);return 1;}
    if(listen(s,1)<0){perror("listen");close(s);return 1;}
    printf("Listening %u\n",port);

    for(;;){
        int c=accept(s,NULL,NULL); if(c<0) break;
        printf("Connected\n");
        char buf[B];
        if(server_first){
            while(fgets(buf,B,stdin)){
                size_t len=strcspn(buf,"\n"); buf[len]=0; if(len==0) continue;
                size_t off=0; while(off<len){ssize_t w=send(c,buf+off,len-off,0); if(w<=0) goto dc; off+=w;}
                ssize_t n=recv(c,buf,B-1,0); if(n<=0) goto dc; buf[n]=0; printf("Received: %s\n",buf);
            }
        }else{
            for(;;){
                ssize_t n=recv(c,buf,B-1,0); if(n<=0) break;
                buf[n]=0; printf("Received: %s\n",buf);
                size_t off=0; while(off<(size_t)n){ssize_t w=send(c,buf+off,n-off,0); if(w<=0) goto dc; off+=w;}
                printf("Echoed: %s\n",buf);
            }
        }
    dc:
        close(c); printf("Client disconnected\n");
    }
    close(s); return 0;
}

//https://chatgpt.com/c/68ca81fa-bd34-8329-b43d-8b4d200b145d
