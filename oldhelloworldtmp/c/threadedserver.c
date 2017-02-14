#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> // write
#include <sys/socket.h>
#include <arpa/inet.h> // inet_addr
#include <pthread.h> // for threading, link with '-lpthread'

void *connection_handler(void *);

int main(int argc,char *argv[]){
	int socket_desc,new_socket,c,*new_sock;
	struct sockaddr_in server,client;
	char *message;
	char *client_ip = inet_ntoa(client.sin_addr);
	int client_port = ntohs(client.sin_port);

	// create socket
	if((socket_desc = socket(AF_INET,SOCK_STREAM,0)) == -1){
		printf("could not create socket");
		return 1;
	}

	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(8888);

	// bind
	if(bind(socket_desc,(struct sockaddr *)&server,sizeof(server)) < 0){
		puts("bind failed");
		return 1;
	}
	puts("bind done");

	// listen
	listen(socket_desc,3);

	// accept incomming connections
	puts("waiting for connections..");
	c = sizeof(struct sockaddr_in);
	while((new_socket = accept(socket_desc,(struct sockaddr *)&client,(socklen_t*)&c))){
		printf("connection accepted from %s:%d\n",client_ip,client_port);

		// send message to client
		message = "Hello, thank you for connecting.\n";
		if(write(new_socket,message,strlen(message)) < 0){
			puts("write failed");
			return 1;
		}
//		sleep(2);
//		close(new_socket);

		pthread_t sniffer_thread;
		new_sock = malloc(1);
		*new_sock = new_socket;
		if(pthread_create(&sniffer_thread,NULL,connection_handler,(void*)new_sock) < 0){
			perror("could not create sniffer thread");
			return 1;
		}
		puts("handler assigned");
	}
	if(new_socket < 0){
		puts("accept failed");
		return 1;
	}

	return 0;
}

// connection handling function
void *connection_handler(void *socket_desc){
	// get socket descriptor
	int sock = *(int*)socket_desc;
	int read_size;
	char *message,client_message[2000];

	message = "Hello, I'll be handling your connection today.\n";
	if(write(sock,message,strlen(message)) < 0){
		puts("connection handler failed to write");
	}
	
	while((read_size = recv(sock,client_message,2000,0)) > 0){
		// send message back to client
		write(sock,client_message,strlen(client_message));
	}
	if(read_size == 0){
		puts("client disconnected");
		fflush(stdout);
	}
	else if(read_size == -1){
		perror("recv failed");
	}

	free(socket_desc);

	return 0;
}
