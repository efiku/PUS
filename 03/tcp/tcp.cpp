#define HAVE_REMOTE
#include "pcap.h"
#include "../lib.h"

#include <iostream>

#pragma comment(lib, "wpcap.lib")
#pragma comment(lib, "ws2_32.lib")
#pragma warning(disable : 4701)

int main(int argc, char** argv)
{
	pcap_if_t *alldevs;
	pcap_if_t *d;
	char errbuf[PCAP_ERRBUF_SIZE];
	pcap_t *device;
	struct addrinfo hints; 
	struct addrinfo *rp, *result; 
	struct in_addr addr;
	unsigned char datagram[sizeof(struct ip) + sizeof(struct udphdr) + sizeof(struct phdr)];
	struct tcphdr *tcp = (struct tcphdr *)(datagram + sizeof(struct ip));
	struct ip *iphdr =  (struct ip *)datagram;
	struct phdr *pseudo_header = (struct phdr *)(datagram + sizeof(struct ip) + sizeof(struct udphdr));

	int i = 0;

	if(pcap_findalldevs_ex(PCAP_SRC_IF_STRING, NULL, &alldevs, errbuf))
	{
		fprintf(stderr,"Error in pcap_findalldevs_ex: %s\n", errbuf);
		exit(1);
	}

	for(d= alldevs; d != NULL; d= d->next)
	{
		printf("%d. %s", ++i, d->name);
		if (d->description)
			printf(" (%s)\n", d->description);
		else
			printf(" (No description available)\n");
	}

	if (i == 0)
	{
		printf("\nNo interfaces found! Make sure WinPcap is installed.\n");
		return 0;
	}

	std::cout<<"Select device: ";
	int dev;
	std::cin>>dev;
	
	if(dev > i)
	{
		std::cout<<"Wrong number"<<std::endl;
		std::cin.get();
		return 0;
	}

	for(d=alldevs, i=0; i< dev-1 ;d=d->next, i++);

	if((device = pcap_open(d->name, 60, PCAP_OPENFLAG_PROMISCUOUS, 1000, NULL, errbuf)) == NULL)
	{
		fprintf(stderr,"\nUnable to open the adapter. %s is not supported by WinPcap\n", d->name);
		std::cin.get();
        return 0;
	}

	memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family     =   AF_INET;        /* Domena komunikacyjna (IPv4). */
    hints.ai_socktype   =   SOCK_RAW;       /* Typ gniazda. */
    hints.ai_protocol   =   IPPROTO_TCP;    /* Protokol. */

	getaddrinfo(argv[1], NULL, &hints, &result);

	addr.S_un.S_addr = ((struct sockaddr_in *)(result->ai_addr))->sin_addr.s_addr;

	tcp->th_sport = htons(rand() % 3000 + 1024);
	tcp->th_seq = tcp->th_sport = htons(rand());
	tcp->th_dport = htons((u_short)atoi(argv[2]));
	tcp->th_ack = 0;
	tcp->th_off = 0;
	tcp->th_sum = internet_checksum((unsigned short *)tcp, sizeof(struct tcphdr) + sizeof(struct phdr));
	tcp->th_win = 512;
	tcp->th_flags = TH_SYN;

	iphdr->ip_dst = addr;
	iphdr->ip_hl = 20;
	iphdr->ip_id = 0;
	iphdr->ip_len = 0;
	iphdr->ip_off = 0;
	iphdr->ip_p = IPPROTO_TCP;
	iphdr->ip_src.S_un.S_addr = inet_addr("192.168.0.1");
	iphdr->ip_sum = internet_checksum((unsigned short *)tcp, sizeof(struct tcphdr) + sizeof(struct phdr));
	iphdr->ip_tos = 0;
	iphdr->ip_v = 4;
	iphdr->ip_ttl = 128;

	while(true)
	{
		tcp->th_sport = htons(rand() % 3000 + 1024);
		tcp->th_seq = tcp->th_sport = htons(rand());
		iphdr->ip_sum = internet_checksum((unsigned short *)tcp, sizeof(struct tcphdr) + sizeof(struct phdr));
		pcap_sendpacket(device, datagram, sizeof(ip) + sizeof(tcphdr));
		Sleep(1000);
		std::cout<<"Send"<<std::endl;
	}

	std::cin.get();

	/* We don't need any more the device list. Free it */
	pcap_freealldevs(alldevs);

	return 0;
}