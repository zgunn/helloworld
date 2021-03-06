#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in_systm.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <netinet/ip_icmp.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

unsigned short in_cksum(unsigned short *addr,int len){
	int nleft = len;
	int sum = 0;
	unsigned short *w = addr;
	unsigned short answer = 0;

	while(nleft > 1){
		sum += *w++;
		nleft -= 2;
	}
	if(nleft == 1){
		*(unsigned char *)(&answer) = *(unsigned char *)w;
		sum += answer;
	}

	sum = (sum >> 16) + (sum & 0xFFFF);
	sum += (sum >> 16);
	answer = ~sum;

	return(answer);
}

int main(int argc,char **argv){
	struct ip ip;
	struct udphdr udp;
	struct icmp icmp;
	const int on = 1;
	struct sockaddr_in sin;
	u_char *packet;
	int sd;

	packet = (u_char *)malloc(60);

	char source_ip[32];
	char dest_ip[32];

	if(argc < 3){
		printf("usage : %s <source ip> <dest ip>\n",argv[0]);
		exit(1);
	}

	strcpy(source_ip,argv[1]);
	strcpy(dest_ip,argv[2]);

	ip.ip_hl = 0x5;
	ip.ip_v = 0x4;
	ip.ip_tos = 0x0;
	ip.ip_len = htons(60);
	ip.ip_id = htons(12830);
	ip.ip_off = 0x0;
	ip.ip_ttl = 64;
	ip.ip_p = IPPROTO_ICMP;
	ip.ip_sum = 0x0;
	ip.ip_src.s_addr = inet_addr(source_ip);
	ip.ip_dst.s_addr = inet_addr(dest_ip);
	ip.ip_sum = in_cksum((unsigned short *)&ip,sizeof(ip));

	memcpy(packet,&ip,sizeof(ip));

	icmp.icmp_type = ICMP_ECHO;
	icmp.icmp_code = 0;
	icmp.icmp_id = 1000;
	icmp.icmp_seq = 0;
	icmp.icmp_cksum = 0;
	icmp.icmp_cksum = in_cksum((unsigned short *)&icmp,8);

	memcpy(packet+20,&icmp,8);

	if((sd = socket(AF_INET,SOCK_RAW,IPPROTO_RAW)) < 0){
		perror("socket failed");
		exit(1);
	}

	if(setsockopt(sd,IPPROTO_IP,IP_HDRINCL,&on,sizeof(on)) < 0){
		perror("setsockopt failed");
		exit(1);
	}

	memset(&sin,0,sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = ip.ip_dst.s_addr;

	if(sendto(sd,packet,60,0,(struct sockaddr *)&sin,sizeof(struct sockaddr)) < 0){
		perror("sednto failed");
		exit(1);
	}

	return 0;
}
