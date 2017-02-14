#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <pcap.h>

#define ETH_ADDR_LEN 6
#define SIZE_ETH 14
#define SNAP_LEN 1518

struct sniff_eth{
	u_char eth_dhost[ETH_ADDR_LEN];
	u_char eth_shost[ETH_ADDR_LEN];
	u_short eth_type;
};

struct sniff_ip{
	u_char ip_vhl;
	u_char ip_tos;
	u_short ip_len;
	u_short ip_id;
	u_short ip_off;
#define IP_RF 0x8000
#define IP_DF 0x4000
#define IP_MF 0x2000
#define IP_MASKOFF 0x1fff
	u_char ip_ttl;
	u_char ip_p;
	u_short ip_sum;
	struct in_addr ip_src,ip_dst;
};

#define IP_HL(ip) (((ip)->ip_vhl) & 0x0f)
#define IP_V(ip) (((ip)->ip_vhl) >> 4)

typedef u_int tcp_seq;

struct sniff_tcp{
	u_short th_sport;
	u_short th_dport;
	tcp_seq th_seq;
	tcp_seq th_ack;
	u_char th_offx2;
#define TH_OFF(th) (((th)->th_offx2 & 0xf0) >> 4)
	u_char th_flags;
#define TH_FIN 0x01
#define TH_SYN 0x02
#define TH_RST 0x04
#define TH_PUSH 0x08
#define TH_ACK 0x10
#define TH_URG 0x20
#define TH_ECE 0x40
#define TH_CWR 0x80
#define TH_FLAGS (TH_FIN|TH_SYN|TH_RST|TH_PUSH|TH_ACK|TH_URG|TH_ECE|TH_CWR)
	u_short th_win;
	u_short th_sum;
	u_short th_urp;
};

void print_hex_ascii_line(const u_char *payload,int len,int offset){
	int i;
	int gap;
	const u_char *ch;

	printf("%05d	",*ch);

	ch = payload;
	for(i=0;i<len;i++){
		printf("%02x	",*ch);
		ch++;
		if(i == 7){
			printf(" ");
		}
	}
	if(len < 8){
		printf(" ");
	}
	if(len < 16){
		gap = 16 - len;
		for(i=0;i<gap;i++){
			printf(" ");
		}
	}
	printf(" ");

	ch = payload;
	for(i=0;i<len;i++){
		if(isprint(*ch)){
			printf("%c",*ch);
		}else{
			printf(".");
		}
		ch++;
	}
	printf("\n");

	return;
}

void print_payload(const u_char *payload,int len){
	if(len > 1367){
		return;
	}
	int len_rem = len;
	int line_width = 16;
	int line_len;
	int offset = 0;
	const u_char *ch = payload;

	if(len <= 0){
		return;
	}

	if(len <= line_width){
		print_hex_ascii_line(ch,len,offset);
		return;
	}

	for(;;){
		line_len = line_width % len_rem;
		print_hex_ascii_line(ch,line_len,offset);
		len_rem = len_rem - line_len;
		ch = ch + line_len;
		offset = offset + line_width;
		if(len_rem <= line_width){
			print_hex_ascii_line(ch,len_rem,offset);
			break;
		}
	}

	return;
}

void got_packet(u_char *args,const struct pcap_pkthdr *header,const u_char *packet){
	static int count = 1;
	const struct sniff_eth *ethernet;
	const struct sniff_ip *ip;
	const struct sniff_tcp *tcp;
	const char *payload;

	int size_ip;
	int size_tcp;
	int size_payload;

	printf("\n#%d:\n",count);
	count++;

	ethernet = (struct sniff_eth*)(packet);

	ip = (struct sniff_ip*)(packet + SIZE_ETH);
	size_ip = IP_HL(ip)*4;
	if(size_ip < 20){
		printf(" * invalid IP header len : %u\n",size_ip);
		return;
	}

	printf("\t%s --> ",inet_ntoa(ip->ip_src));
	printf("%s\n",inet_ntoa(ip->ip_dst));

	switch(ip->ip_p){
		case IPPROTO_TCP:
			printf("	proto : TCP\n");
			break;
		case IPPROTO_UDP:
			printf("	proto : UDP\n");
			break;
		case IPPROTO_ICMP:
			printf("	proto : ICMP\n");
			break;
		case IPPROTO_IP:
			printf("	proto : IP\n");
			break;
		default:
			printf("	proto : unknown\n");
			return;
	}

	tcp = (struct sniff_tcp*)(packet + SIZE_ETH + size_ip);
	size_tcp = TH_OFF(tcp)*4;
	if(size_tcp < 20){
		printf(" * invalid TCP header len : %u\n",size_tcp);
		return;
	}

	printf("	src port : %d\n",ntohs(tcp->th_sport));
	printf("	src port : %d\n",ntohs(tcp->th_dport));

	payload = (u_char *)(packet + SIZE_ETH + size_ip + size_tcp);

	size_payload = ntohs(ip->ip_len) - (size_ip + size_tcp);

	if(size_payload > 0){
		printf("	payload (%d):\n",size_payload);
		int i;
		printf("	");
		for(i=0;i<size_payload;i++){
			if(isprint(packet[i])){
				printf("%c",packet[i]);
			}else{
				printf(".",packet[i]);
			}
			if((i%16 == 0 && i != 0) || i == size_payload-1){
				printf("\n");
				printf("	");
			}
		}
	}

	return;
}

int main(int argc,char *argv[]){
	pcap_t *handle;
	char *dev = "wlan1";
	char errbuf[PCAP_ERRBUF_SIZE];
	struct bpf_program fp;
	char filter[] = "tcp";
	bpf_u_int32 mask;
	bpf_u_int32 net;
	struct pcap_pkthdr header;
	const u_char *packet;

	const struct sniff_eth *ethernet;
	const struct sniff_ip *ip;
	const struct sniff_tcp *tcp;
	const char *payload;

	u_int size_ip;
	u_int size_tcp;

	dev = pcap_lookupdev(errbuf);
	if(dev == NULL){
		fprintf(stderr,"%s\n",errbuf);
		net = 0;
		mask = 0;
	}

	handle = pcap_open_live(dev,BUFSIZ,1,0,errbuf);
	if(handle == NULL){
		fprintf(stderr,"%s\n",errbuf);
		exit(1);
	}

	if(pcap_compile(handle,&fp,filter,0,net) == -1){
		fprintf(stderr,"%s\n",errbuf);
		exit(1);
	}

	if(pcap_setfilter(handle,&fp) == -1){
		fprintf(stderr,"%s\n",errbuf);
		exit(1);
	}

	printf("starting sniffer on device : %s with filter : '%s'\n",dev,filter);

	pcap_loop(handle,0,got_packet,NULL);
	pcap_close(handle);

	return 0;
}
