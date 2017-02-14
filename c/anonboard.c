#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h> /* write() */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h> /* inet_addr() */
#include <pthread.h> /* for threading, link with lpthread */


#define LISTEN_PORT 2000

void error(const char *msg){ /* const qualifier indicates no intent to change variable value */
    if(errno){
        printf("%s : %s\n",msg,strerror(errno));
    }else{
        printf("%s\n",msg);
    }
    exit(1);
}

void *connection_handler(void *); /* function handles individual threads */

int main(int argc,char *argv[]){
    int sockd,client_sockd;
    int c = sizeof(struct sockaddr_in);
    int reusaddr = 1;
    struct sockaddr_in sin,client_sin;
    pthread_t thread_id;

    if((sockd = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP)) < 0){
        error("socket()");
    }
    if(setsockopt(sockd,SOL_SOCKET,SO_REUSEADDR,&reusaddr,sizeof(int)) < 0){
        error("setsockopt()");
    }

    sin.sin_family = AF_INET;
    sin.sin_port = htons(LISTEN_PORT);
    sin.sin_addr.s_addr = INADDR_ANY;

    if(bind(sockd,(struct sockaddr *)&sin,sizeof(sin)) < 0){
        error("bind()");
    }
    if(listen(sockd,5) < 0){
        error("listen()");
    }

    while((client_sockd = accept(sockd,(struct sockaddr*)&client_sin,(socklen_t*)&c)) >= 0){
        printf("Got connection from %s:%d\n",inet_ntoa(client_sin.sin_addr),ntohs(client_sin.sin_port));
        if(pthread_create(&thread_id,NULL,connection_handler,(void*)&client_sockd) < 0){
            error("pthread_create()");
        }
        printf("Handler assigned\n");
    }

    if(client_sockd < 0){
        error("accept()");
    }

    return 0;
}

int post(FILE *fp,char msg[],int sd){
    printf("Client posting.\n");
    char name[64];
    char prompt[1024];

    if(fp == NULL){
        printf("post() fp == NULL\n");
        return 1;
    }
    if(sd < 0){
        printf("post() sd < 0\n");
        return 1;
    }
    strcpy(name,"Anon");

    memset(msg,0,2000);

    strcpy(prompt,"Please type your post. End with ~ on a new line.\n");
    write(sd,prompt,strlen(prompt));
    fprintf(fp,"[%s]\n",name);
    while(msg[0] != '~'){
        memset(msg,0,2000);
        recv(sd,msg,2000,0);
        fprintf(fp,"---%s",msg);
    }
    strcpy(prompt,"Post successful!\n");
    write(sd,prompt,strlen(prompt));
    fclose(fp);

    return 0;
}

void *connection_handler(void *socketd){
    int sd = *(int*)socketd;
    int read_siz;
    char *banner = "*[v]iew board\n*[p]ost to board\n";
    char buf[2048];
    char client_msg[2000];
    FILE *fp;
    char *filename = "board.txt";

    write(sd,banner,strlen(banner)); /* send banner */

    while((read_siz = recv(sd,client_msg,2000,0)) > 0){
        client_msg[read_siz] = '\0'; /* EOL marker */
        if(client_msg[0] == 'p'){
            fp = fopen(filename,"a");

            memset(client_msg,0,2000);
            post(fp,client_msg,sd);
        }
        if(client_msg[0] == 'v'){
            printf("Client requested %s.\n",filename);
            fp = fopen(filename,"r");
            while(fgets(buf,sizeof(buf),fp) != 0){
                write(sd,buf,strlen(buf));
            }
            fclose(fp);
        }
        memset(client_msg,0,2000); /* clear message buffer */
    }
    if(read_siz == 0){
        printf("Client disconnected\n");
        fflush(stdout);
    }
    if(read_siz < 0){
        error("recv()");
    }
    return 0;
}
