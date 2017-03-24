#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")

int connected = 1;

int main(int argc,char *argv[]){
	ShowWindow(GetConsoleWindow(),SW_HIDE); // should hide console window
	SOCKET sd;
	struct sockaddr_in sin;
	WSADATA wsaData;
	char rbuf[1024];
	char sbuf[1024];

	char dest_ip[100];
	int dest_port = 80;
	int readsiz;

	if(WSAStartup(MAKEWORD(2,2),&wsaData) != 0){
		printf("WSAStartup\n");
		exit(1);
	}
	
	if((sd = socket(AF_INET,SOCK_STREAM,0)) == INVALID_SOCKET){
		printf("socket\n");
		exit(1);
	}
	strcpy(dest_ip,"107.15.35.153");

	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = inet_addr(dest_ip);
	sin.sin_port = htons(dest_port);

	if(connect(sd,(struct sockaddr*)&sin,sizeof(sin)) < 0){
		printf("connect\n");
		exit(1);
	}

	printf("connected\n");
	send(sd,"test",4,0);

	while((readsiz = recv(sd,rbuf,sizeof(rbuf),0)) > 0){
		printf("got here\n");
		printf("%d\n",readsiz);
		rbuf[readsiz] = '\0';
	}

	printf("closing\n");
	closesocket(sd);
	WSACleanup();

	exit(0);
}
