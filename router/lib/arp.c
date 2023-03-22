#include "router.h"

int arp_replay(Eth_hdr *eth_hdr, ARP_hdr *arp_hdr, int interface)
{
	char* ip = get_interface_ip(interface);
	printf("%s\n", ip);

	memcpy(&arp_hdr->tha, &arp_hdr->sha, arp_hdr->hlen);
	get_interface_mac(interface, arp_hdr->sha);

	arp_hdr->op = htons(2);

	uint32_t adr_tmp = arp_hdr->tpa;
	arp_hdr->tpa = arp_hdr->spa;
	arp_hdr->spa = adr_tmp;

	memcpy(&eth_hdr->ether_dhost, &arp_hdr->tha, arp_hdr->hlen);
	memcpy(&eth_hdr->ether_shost, &arp_hdr->sha, arp_hdr->hlen);
	return 1;
}

int arp_request(Eth_hdr *eth_hdr, RTable_entry *next)
{
	ARP_entry arp_entry;
	char buf[MAX_PACKET_LEN];

	Eth_hdr *eth_hdr_new = (Eth_hdr *)(buf);

	printf("Writing addess to eth header\n");
	get_interface_mac(next->interface, eth_hdr->ether_shost);
	memcpy(&eth_hdr->ether_dhost, &arp_entry.mac, sizeof(arp_entry.mac));
	return 0;
}
