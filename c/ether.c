#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <netinet/ether.h>
#include <linux/if_packet.h>

#define DST_MAC0 0x00
#define DST_MAC1 0x01
#define DST_MAC2 0x02
#define DST_MAC3 0x03
#define DST_MAC4 0x04
#define DST_MAC5 0x05

unsigned short csum(unsigned short *buf,int nwords);

int main(int argc,char *argv[]){
	srand(time(0));
	int sock;
	if((sock = socket(AF_PACKET,SOCK_RAW,IPPROTO_RAW)) < 0){
		perror("socket");
		exit(1);
	}

	struct ifreq if_idx;
	memset(&if_idx,0,sizeof(struct ifreq));
	strncpy(if_idx.ifr_name,"wlan1",IFNAMSIZ-1);
	if(ioctl(sock,SIOCGIFINDEX,&if_idx) < 0){
		perror("SIOCGIFINDEX");
		exit(1);
	}

	struct ifreq if_mac;
	memset(&if_mac,0,sizeof(if_mac));
	strncpy(if_mac.ifr_name,"wlan1",IFNAMSIZ-1);
	if(ioctl(sock,SIOCGIFHWADDR,&if_mac) < 0){
		perror("SIOCGIFHWADDR");
		exit(1);
	}

	struct ifreq if_ip;
	memset(&if_ip,0,sizeof(if_ip));
	strncpy(if_ip.ifr_name,"wlan1",IFNAMSIZ-1);
	if(ioctl(sock,SIOCGIFADDR,&if_ip) < 0){
		perror("SIOCGIFADDR");
		exit(1);
	}

	int tx_len = 0;
	char sendbuf[1024];
	struct ether_header *eh = (struct ether_header *)sendbuf;
	memset(sendbuf,0,1024);
	/* sends from actual MAC address of interface
	eh->ether_shost[0] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[0];
	eh->ether_shost[1] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[1];
	eh->ether_shost[2] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[2];
	eh->ether_shost[3] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[3];
	eh->ether_shost[4] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[4];
	eh->ether_shost[5] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[5]; */
	eh->ether_shost[0] = 0x10;
	eh->ether_shost[1] = 0x20;
	eh->ether_shost[2] = 0x30;
	eh->ether_shost[3] = 0x40;
	eh->ether_shost[4] = 0x50;
	eh->ether_shost[5] = 0x60;
	eh->ether_dhost[0] = DST_MAC0;
	eh->ether_dhost[1] = DST_MAC1;
	eh->ether_dhost[2] = DST_MAC2;
	eh->ether_dhost[3] = DST_MAC3;
	eh->ether_dhost[4] = DST_MAC4;
	eh->ether_dhost[5] = DST_MAC5;

	eh->ether_type = htons(ETH_P_IP);
	tx_len += sizeof(struct ether_header);

	struct iphdr *ip = (struct iphdr *)(sendbuf+sizeof(struct ether_header));

	if(argc != 2){
		printf("usage : %s <dest ip>\n",argv[0]);
		exit(1);
	}

	char dest_ip[32];
	strncpy(dest_ip,argv[1],sizeof(dest_ip));

	ip->ihl = 5;
	ip->version = 4;
	ip->tos = 16;
	ip->id = htons(rand()%30000+60000);
	ip->ttl = 64;
	ip->protocol = 17;
	ip->saddr = inet_addr(inet_ntoa(((struct sockaddr_in *)&if_ip.ifr_addr)->sin_addr));
	ip->daddr = inet_addr(dest_ip); 
	tx_len += sizeof(struct iphdr);

	struct udphdr *udp = (struct udphdr *)(sendbuf+sizeof(struct iphdr)+sizeof(struct ether_header));

	udp->source = htons(rand()%1025+30000);
	udp->dest = htons(5342);
	udp->check = 0;
	tx_len += sizeof(struct udphdr);

	sendbuf[tx_len++] = 0xde;
	sendbuf[tx_len++] = 0xaf;
	sendbuf[tx_len++] = 0xc0;
	sendbuf[tx_len++] = 0xde;

	udp->len = htons(tx_len - sizeof(struct ether_header) - sizeof(struct iphdr));
	ip->tot_len = htons(tx_len - sizeof(struct ether_header));
	ip->check = csum((unsigned short *)(sendbuf+sizeof(struct ether_header)),sizeof(struct iphdr)/2);

	struct sockaddr_ll socket_address;

	socket_address.sll_ifindex = if_idx.ifr_ifindex;
	socket_address.sll_halen = ETH_ALEN;
	socket_address.sll_addr[0] = DST_MAC0;
	socket_address.sll_addr[1] = DST_MAC1;
	socket_address.sll_addr[2] = DST_MAC2;
	socket_address.sll_addr[3] = DST_MAC3;
	socket_address.sll_addr[4] = DST_MAC4;
	socket_address.sll_addr[5] = DST_MAC5;

	if(sendto(sock,sendbuf,tx_len,0,(struct sockaddr*)&socket_address,sizeof(struct sockaddr_ll)) < 0){
		perror("sendto");
		exit(1);
	}

	return 0;
}

unsigned short csum(unsigned short *buf,int nwords){
	unsigned long sum;

	for(sum=0;nwords>0;nwords--){
		sum += *buf++;
	}

	sum = (sum >> 16)+(sum & 0xFFFF);
	sum += (sum >> 16);
	return(unsigned short)(~sum);
}
