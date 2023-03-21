#include "queue.h"
#include "lib.h"
#include "protocols.h"
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>

typedef struct ether_header Eth_hdr;
typedef struct iphdr IP_hdr;
typedef struct route_table_entry RTable_entry;
typedef struct arp_header ARP_hdr;
typedef struct arp_entry ARP_entry;

#define ARP_TYPE 0x0806
#define IPV4_TYPE 0x0800
#define PREFIX_MASK 4294967040

// int arp_replay(void * buffer, int interface, int buff_len);

// void convert_to_host(void *buf_addr, long len) {
// 	uint8_t *to_convert = ((u_int8_t*)buf_addr);
// 	for (int i = 0; i < len/2; i++)
// 	{
// 		uint8_t tmp = to_convert[len - i - 1];
// 		to_convert[len - 1 - i] = to_convert[i];
// 		to_convert[i] = tmp;
// 	}
// }



int is_for_this_mac(int interface, Eth_hdr *eth_hdr) {
	uint8_t *router_mac_addr = malloc(sizeof(eth_hdr->ether_dhost));
	uint8_t *eth_dest_toh = malloc(sizeof(eth_hdr->ether_dhost));
	memcpy(eth_dest_toh, eth_hdr->ether_dhost, sizeof(eth_hdr->ether_dhost));

	get_interface_mac(interface, router_mac_addr);

	for (int i = 0; i < sizeof(eth_hdr->ether_dhost); i++)
	{
		if (eth_dest_toh[i] != router_mac_addr[i]) {
			free(router_mac_addr);
			free(eth_dest_toh);
			return 0;
		}
	}
	free(router_mac_addr);
	free(eth_dest_toh);
	return 1;
}

void print_ip(uint32_t ip) {
	for (int i = 0; i < 4; i++)
	{
		printf("%hhu.", *((uint8_t*)(&ip) + sizeof(uint8_t)*i));
	}
	printf("   %u\n", ip);
}

int is_same_checksum(uint16_t oldcheck, IP_hdr* ip_hdr) {
	ip_hdr->check = 0;
	uint16_t check = checksum((uint16_t *)ip_hdr, sizeof(IP_hdr));

	if (check != oldcheck) {
		return 0;
	}
	return 1;
}

RTable_entry* find_next_route(RTable_entry* table, int len, uint32_t ip_daddr) {
	
	int pos = 0, first = 0, last = len;
	RTable_entry *next_route = NULL;
	uint32_t max_mask = 0;

	
	while(last - first > 1) {
		pos = (first + last)/2;
		uint32_t prefix = ntohl(table[pos].prefix);
		uint32_t mask = ntohl(table[pos].mask);
		uint32_t searched_ip = (ip_daddr & PREFIX_MASK);
		// print_ip(table[pos].prefix);

		
		if (searched_ip == prefix) {
			// printf("\n\nFound Prefix : ");print_ip(prefix);
			// printf("IP + mask : ");print_ip(searched_ip);

			while (searched_ip == table[pos + 1].prefix) {
				searched_ip = (ip_daddr & mask);
				pos++;
			}
			if (mask > max_mask) {
				next_route = &table[pos];
				max_mask = mask;
			}
			first = pos;
			continue;
			
		} else if (searched_ip > prefix) {
			first = pos + 1;
		} else {
			last = pos;
		}
	}
	
	return next_route;
}

int compare_rtables_entry(const void *a ,const void *b) {
	RTable_entry a_entry = *(RTable_entry *)a;
	RTable_entry b_entry = *(RTable_entry *)b;
	uint32_t a_en = ntohl(a_entry.prefix);
	uint32_t b_en = ntohl(b_entry.prefix);
	uint32_t a_mask = ntohl(a_entry.mask);
	uint32_t b_mask = ntohl(b_entry.mask);
	

	if (a_en != b_en) {
		return a_en - b_en;
	} else {
		return a_mask - b_mask;
	}
}

void print_mac(uint8_t* mac) {
	for (int i = 0; i < 6; i++)
	{
		printf("%x:", mac[i]);
	}
	printf("\n");
}

int main(int argc, char *argv[])
{
	char buf[MAX_PACKET_LEN];

	// Do not modify this line
	init(argc - 2, argv + 2);

	RTable_entry* rtable = malloc(sizeof(RTable_entry) * 100000);
	int rt_size = read_rtable(argv[1], rtable);
	qsort(rtable, rt_size, sizeof(RTable_entry), compare_rtables_entry);

	ARP_entry* arp_table = malloc(sizeof(ARP_entry) * 36);
	int arpt_size = parse_arp_table("arp_table.txt", arp_table);

	while (1) {

		int interface;
		size_t len;

		interface = recv_from_any_link(buf, &len);
		DIE(interface < 0, "recv_from_any_links");
		printf("Recived a packet\n");

		Eth_hdr *eth_hdr = (Eth_hdr *) buf;
		printf("Dest :"); print_mac((eth_hdr->ether_dhost));
		printf("Source :"); print_mac((eth_hdr->ether_shost));

		/* Note that packets received are in network order,
		any header field which has more than 1 byte will need to be conerted to
		host order. For example, ntohs(eth_hdr->ether_type). The oposite is needed when
		sending a packet on the link, */
		
		uint16_t eth_type = ntohs(eth_hdr->ether_type);
		
		if (eth_type == ARP_TYPE) {
			
			// ARP PROTOCOL
			printf("ARP SEARCH and REPLAY\n");

			ARP_hdr* arp_hdr = (ARP_hdr *)(buf + sizeof(Eth_hdr));
			memcpy(&arp_hdr->tha, &arp_hdr->sha, sizeof(arp_hdr->sha));
			get_interface_mac(interface, arp_hdr->sha);

			uint16_t op_replay = htons(2);
			memcpy(&arp_hdr->op, &op_replay, sizeof(uint16_t));
			
			uint32_t adr_tmp = arp_hdr->tpa;
			memcpy(&arp_hdr->tpa, &arp_hdr->spa, sizeof(uint32_t));
			memcpy(&arp_hdr->spa, &adr_tmp, sizeof(uint32_t));

			memcpy(&eth_hdr->ether_dhost, &arp_hdr->tha, sizeof(arp_hdr->tha));
			memcpy(&eth_hdr->ether_shost, &arp_hdr->sha, sizeof(arp_hdr->sha));

		} else if (eth_type == IPV4_TYPE) {

			// IPv4 PROTOCOL 

			// Check mac
			if(!is_for_this_mac(interface, eth_hdr)) {
				printf("Not for me mac\n");
				continue;
			}
			
			IP_hdr* ip_hdr = (IP_hdr *)(buf + sizeof(Eth_hdr));
			uint16_t old_check = ntohs(ip_hdr->check);

			// Check checksy=um
			if (!is_same_checksum(old_check, ip_hdr)) {
				printf("The checksum differs\n");
				continue;
			}

			// Check ttl
			if (ip_hdr->ttl < 2) {
				printf("TTL Exipred\n");
				// TODO: send back ICMP Time Exceeded
				continue;
			} else {
				ip_hdr->ttl--;
			}

			// Find next route
			RTable_entry *next = find_next_route(rtable, rt_size, ntohl(ip_hdr->daddr));

			if (next != NULL) {
				printf("\n\n\n");
				
				printf("Prefix : ");print_ip(next->prefix);
				printf("Next Hop : ");print_ip(next->next_hop);
				printf("Mask : ");print_ip(next->mask);
				printf("Interface %d", next->interface);
			
				printf("\n\n\n");

			} else {
				// printf("Next null\n");
				// TODO send back ICMP Destination Unreachble
				continue;
			}

			// Recalulate checksum
			// ip_hdr->check = ~(~old_check + ~((uint16_t)ip_hdr->ttl + 1) + (uint16_t)ip_hdr->ttl) - 1;
			ip_hdr->check = htons(checksum((uint16_t *)ip_hdr, sizeof(IP_hdr)));

			// Write address to frame
			for (int i = 0; i < arpt_size; i++)
			{
				if (arp_table[i].ip == next->next_hop) {
					printf("Writing addess to eth header\n");
					interface = next->interface;
					get_interface_mac(next->interface, eth_hdr->ether_shost);
					memcpy(&eth_hdr->ether_dhost, &arp_table[i].mac, sizeof(arp_table[i].mac));
					break;
				}
			}
			
		}
		print_mac(eth_hdr->ether_dhost);
		print_mac(eth_hdr->ether_shost);
		// Send to link
		send_to_link(interface, buf, len);
	}

	free(rtable);

}

