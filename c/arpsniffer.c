#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pcap.h>

#define ARP_REQ 1
#define ARP_REPL 2

typedef struct arphdr {
    u_int16_t htype;
    u_int16_t ptype;
    u_char hlen;
    u_char plen;
    u_int16_t oper;
    u_char sha[6];
    u_char spa[4];
    u_char tha[6];
    u_char tpa[4];
}arphdr_t;

#define MAXCAPT 2048

int main(int argc,char *argv[]){
    int i=0;
    bpf_u_int32 netaddr=0,mask=0; // network addr and netmask
    struct bpf_program filter; // bpf prog filter
    char errbuf[PCAP_ERRBUF_SIZE];
    pcap_t *desc = NULL;
    struct pcap_pkthdr pkthdr; // header of captured packets
    const unsigned char *packet = NULL;
    arphdr_t *arpheader = NULL;
    memset(errbuf,0,PCAP_ERRBUF_SIZE);

    if(argc != 2){
        fprintf(stderr,"usage : %s <iface>\n",argv[0]);
        exit(1);
    }

    if((desc = pcap_open_live(argv[1],MAXCAPT,0,512,errbuf)) == NULL){
        fprintf(stderr,"err : %s\n",errbuf);
        exit(1);
    }

    if(pcap_compile(desc,&filter,"arp",1,mask) == -1){ // pcap filter
        fprintf(stderr,"err : %s\n",pcap_geterr(desc));
        exit(1);
    }

    if(pcap_setfilter(desc,&filter) == -1){
        fprintf(stderr,"err : %s\n",pcap_geterr(desc));
        exit(1);
    }

    while(1){
        if((packet = pcap_next(desc,&pkthdr)) == NULL){
            sleep(2);
            continue;
        }

        arpheader = (struct arphdr *)(packet+14); // pointer to arp header


        printf("\n\npkt size : %db\n",pkthdr.len);
        printf("hw type : %s\n",(ntohs(arpheader->htype) == 1) ? "Ethernet" : "Unknown");
        printf("proto type : %s\n",(ntohs(arpheader->ptype) == 0x0800) ? "IPv4" : "Unknown");
        printf("oper : %s\n",(ntohs(arpheader->oper) == ARP_REQ) ? "ARP request" : "ARP reply");

        if(ntohs(arpheader->htype) == 1 && ntohs(arpheader->ptype) == 0x0800){
            printf("\t");
            for(i=0;i<6;i++){
                printf("%02X:",arpheader->sha[i]);
            }
			printf(" --> ");
			for(i=00;i<6;i++){
				printf("%02X:",arpheader->tha[i]);
			}

            printf("\n\t");
            for(i=0;i<4;i++){
                printf("%d.",arpheader->spa[i]);
            }
			printf(" -- > ");
			for(i=0;i<4;i++){
                printf("%d.",arpheader->tpa[i]);
            }

            printf("\n");
        }
 
    }

    return 0;
}
