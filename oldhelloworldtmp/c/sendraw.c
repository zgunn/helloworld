#include <stdio.h>
#include <stdlib.h>
#include <string.h> // memset
#include <errno.h> // errno
#include <sys/socket.h>
#include <netinet/tcp.h> // tcp header
#include <netinet/ip.h> // ip header

struct pseudo_header { // needed for tcp header checksum calculation
	u_int32_t source_address;
	u_int32_t dest_address;
	u_int8_t placeholder;
	u_int8_t protocol;
	u_int16_t tcp_length;
};

unsigned short csum(unsigned short *ptr,int nbytes){ // generic checksum calculation function
	register long sum;
	unsigned short oddbyte;
	register short answer;

	sum = 0;
	while(nbytes > 1){
		sum += *ptr++;
		nbytes -= 2;
	}
	if(nbytes == 1){
		oddbyte = 0;
		*((u_char*)&oddbyte)=*(u_char*)ptr;
		sum += oddbyte;
	}

	sum = (sum >> 16)+(sum & 0xffff);
	sum = sum + (sum >> 16);
	answer = (short)~sum;

	return(answer);
}

int main(int argc,char *argv[]){
	srand(time(0));
	char *message;
	int socket_desc = socket(PF_INET,SOCK_RAW,IPPROTO_TCP);

	if(socket_desc == -1){
		perror("failed to create socket");
		exit(1);
	}

	// datagram to represent the packet
	char datagram[4096],source_ip[32],*data,*pseudogram;
	memset(datagram,0,4096);

	// ip header
	struct iphdr *iph = (struct iphdr *) datagram;

	// tcp header
	struct tcphdr *tcph = (struct tcphdr *) (datagram + sizeof(struct ip));
	struct sockaddr_in sin;
	struct pseudo_header psh;

	// data part
	data = datagram + sizeof(struct iphdr) + sizeof(struct tcphdr);

	// user args
	char dest_ip[30];
	int dest_port;
	if(argc != 4){
		printf("usage : %s <source ip> <dest ip> <dest port>\n",argv[0]);
		exit(1);
	}

	strcpy(dest_ip,argv[2]);
	dest_port = atoi(argv[3]);

	message = "";
	if(dest_port == 80){
		message = "GET / HTTP/1.1\r\n\r\n";
	}
	if(dest_port == 21){
		message = "USER anonymous";
	}
	if(dest_port == 23){
		message = "Do Suppress Go Ahead";
	}
	strcpy(data,message);

	// address resolution
	strcpy(source_ip,argv[1]);
	sin.sin_family = AF_INET;
	sin.sin_port = htons(80);
	sin.sin_addr.s_addr = inet_addr(dest_ip);

	// fill ip header
	iph->ihl = 5;
	iph->version = 4;
	iph->tos = 0;
	iph->tot_len = sizeof(struct iphdr) + sizeof(struct tcphdr) + strlen(data);
//	iph->id = htonl(54321);
	iph->id = htonl(rand()%65535+1025);
	iph->frag_off = 0;
	iph->ttl = 64;
	iph->protocol = IPPROTO_TCP;
	iph->check = 0;
	iph->saddr = inet_addr(source_ip);
	iph->daddr = sin.sin_addr.s_addr;

	// ip checksum
	iph->check = csum((unsigned short *) datagram,iph->tot_len);

	// fill tcp header
	tcph->source = htons(rand()%65535+30000);
	tcph->dest = htons(dest_port);
	tcph->seq = 0;
	tcph->ack_seq = 0;
	tcph->doff = 5; // tcp header size
	// tcp flags
	tcph->fin = 0;
	tcph->syn = 1;
	tcph->rst = 0;
	tcph->psh = 0;
	tcph->ack = 0;
	tcph->urg = 0;
	tcph->window = htons(5840); // max window size
	tcph->check = 0; // filled later by psuedo header
	tcph->urg_ptr = 0;

	// tcp checksum
	psh.source_address = inet_addr(source_ip);
	psh.dest_address = sin.sin_addr.s_addr;
	psh.placeholder = 0;
	psh.protocol = IPPROTO_TCP;
	psh.tcp_length = htons(sizeof(struct tcphdr) + strlen(data));

	int psize = sizeof(struct pseudo_header) + sizeof(struct tcphdr) + strlen(data);
	pseudogram = malloc(psize);

	memcpy(pseudogram,(char*)&psh,sizeof(struct pseudo_header));
	memcpy(pseudogram + sizeof(struct pseudo_header),tcph,sizeof(struct tcphdr) + strlen(data));

	tcph->check = csum((unsigned short*)pseudogram,psize);

	// IP_HDRINCL tells kernel that headers are included in the packet
	int one = 1;
	const int *val = &one;

	if(setsockopt(socket_desc,IPPROTO_IP,IP_HDRINCL,val,sizeof(one)) < 0){
		perror("error setting IP_HDRINCL");
		exit(1);
	}

	// send packet
	if(sendto(socket_desc,datagram,iph->tot_len,0,(struct sockaddr *)&sin,sizeof(sin)) < 0){
		perror("send failed");
		exit(1);
	}else{
		printf("packet send. length : %d\n",iph->tot_len);
	}

	return 0;
}
