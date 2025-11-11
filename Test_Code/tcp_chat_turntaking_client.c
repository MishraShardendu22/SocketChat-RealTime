// tcp_turn_client.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/socket.h>

#define P 12345
#define B 512

int main(int argc,char **argv){
    if(argc<2){fprintf(stderr,"Usage: %s <IP> [port] [first]\n",argv[0]);return 1;}
    const char *ip=argv[1];
    unsigned short port=P;
    int server_first=0; 
    if(argc>=3){
        if(isdigit((unsigned char)argv[2][0])) port=(unsigned short)atoi(argv[2]);
        else server_first=(argv[2][0]=='s'||argv[2][0]=='S');
    }
    if(argc>=4) server_first=(argv[3][0]=='s'||argv[3][0]=='S');

    signal(SIGPIPE,SIG_IGN);
    int s=socket(AF_INET,SOCK_STREAM,0); if(s<0){perror("socket");return 1;}
    struct sockaddr_in a={.sin_family=AF_INET,.sin_port=htons(port)};
    if(inet_pton(AF_INET,ip,&a.sin_addr)<=0){fprintf(stderr,"Invalid IP\n");close(s);return 1;}
    if(connect(s,(struct sockaddr*)&a,sizeof(a))<0){perror("connect");close(s);return 1;}
    printf("Connected\n");
    char buf[B];

    if(server_first){
        for(;;){
            ssize_t n=recv(s,buf,B-1,0); if(n<=0) break;
            buf[n]=0; printf("Received: %s\n",buf);
            if(fgets(buf,B,stdin)==NULL) break;
            size_t len=strcspn(buf,"\n"); buf[len]=0; if(len==0) continue;
            if(strcmp(buf,"quit")==0) break;
            size_t off=0; while(off<len){ssize_t w=send(s,buf+off,len-off,0); if(w<=0) goto end; off+=w;}
        }
    }else{
        while(fgets(buf,B,stdin)){
            size_t len=strcspn(buf,"\n"); buf[len]=0; if(len==0) continue;
            if(strcmp(buf,"quit")==0) break;
            size_t off=0; while(off<len){ssize_t w=send(s,buf+off,len-off,0); if(w<=0){perror("send");goto end;} off+=w;}
            ssize_t n=recv(s,buf,B-1,0); if(n<=0) break; buf[n]=0; printf("Echo: %s\n",buf);
        }
    }

end:
    close(s); return 0;
}
