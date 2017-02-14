#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#define LISTEN_PORT 2000

void error(const char *msg){
    if(errno){
        printf("%s: %s\n",msg,strerror(errno));
    }else{
        printf("%s\n",msg);
    }
    exit(1);
}

void *con_handler(void *);

int main(int argc,char *argv[]){
    int sd,csd;
    int c = sizeof(struct sockaddr_in);
    int reuseaddr = 1;
    struct sockaddr_in sin,csin;
    pthread_t thread_id;

    if((sd = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP)) < 0){
        error("socket()");
    }
    if(setsockopt(sd,SOL_SOCKET,SO_REUSEADDR,&reuseaddr,sizeof(int)) < 0){
        error("setsockopt()");
    }

    sin.sin_family = AF_INET;
    sin.sin_port = htons(LISTEN_PORT);
    sin.sin_addr.s_addr = INADDR_ANY;

    if(bind(sd,(struct sockaddr *)&sin,sizeof(sin)) < 0){
        error("bind()");
    }
    if(listen(sd,5) < 0){
        error("listen()");
    }

    while((csd = accept(sd,(struct sockaddr*)&csin,(socklen_t*)&c)) >= 0){
        printf("Got connection from %s:%d\n",inet_ntoa(csin.sin_addr),ntohs(csin.sin_port));
        if(pthread_create(&thread_id,NULL,con_handler,(void*)&csd) < 0){
            error("pthread_create()");
        }
        printf("Handler assigned.\n");
    }

    if(csd < 0){
        error("accept()");
    }

    return 0;
}

void *con_handler(void *sockd){
    int sd = *(int*)sockd;
    int readsiz;
    char cmsg[2048];

    while((readsiz = recv(sd,cmsg,2048,0)) > 0){
        cmsg[readsiz] = '\0'; /* EOL marker */
    }
    if(readsiz == 0){
        printf("Client disconnected.\n");
        fflush(stdout);
    }
    if(readsiz < 0){
        error("recv()");
    }

    return 0;
}
