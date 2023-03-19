#include "queue.h"
#include "lib.h"
#include "protocols.h"
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>

// void convert_to_host(void *buf_addr, long len) {
// 	uint8_t *to_convert = ((u_int8_t*)buf_addr);
// 	for (int i = 0; i < len/2; i++)
// 	{
// 		uint8_t tmp = to_convert[len - i - 1];
// 		to_convert[len - 1 - i] = to_convert[i];
// 		to_convert[i] = tmp;
// 	}
// }
typedef struct ether_header Eth_hdr;
typedef struct iphdr IP_hdr;
typedef struct route_table_entry RTable_entry;


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
	print_ip(ip_daddr);
	
	int pos = 0, first = 0, last = len;
	RTable_entry *next_route = NULL;
	
	while(last - first > 1) {
		pos = (first + last)/2;
		uint32_t prefix = ntohl(table[pos].prefix);
		print_ip(prefix);
		// uint8_t pre_tmp = *((uint8_t*)(&prefix) + sizeof(uint8_t)*sub_byte);
		// uint8_t ip_tmp = *((uint8_t*)(&ip_daddr) + sizeof(uint8_t)*sub_byte);
		
		if (ip_daddr == prefix) {
			next_route = &table[pos];
			break;
		} else if (ip_daddr > prefix) {
			first = pos + 1;
		} else {
			last = pos;
		}
	}
	// print_ip(table[pos].prefix);
	return next_route;
}

int main(int argc, char *argv[])
{
	char buf[MAX_PACKET_LEN];

	// Do not modify this line
	init(argc - 2, argv + 2);


	while (1) {

		int interface;
		size_t len;

		interface = recv_from_any_link(buf, &len);
		DIE(interface < 0, "recv_from_any_links");
		printf("Recived a packet\n");

		Eth_hdr *eth_hdr = (Eth_hdr *) buf;
		/* Note that packets received are in network order,
		any header field which has more than 1 byte will need to be conerted to
		host order. For example, ntohs(eth_hdr->ether_type). The oposite is needed when
		sending a packet on the link, */

		if(!is_for_this_mac(interface, eth_hdr)) {
			continue;
		}
		
		IP_hdr* ip_hdr = (IP_hdr *)(buf + sizeof(Eth_hdr));
		uint16_t old_check = ntohs(ip_hdr->check);

		if (!is_same_checksum(old_check, ip_hdr)) {
			printf("The checksum differs\n");
			continue;
		}

		if (ip_hdr->ttl < 2) {
			printf("TTL Exipred\n");
			// TODO: send back ICMP Time Exceeded
			continue;
		} else {
			ip_hdr->ttl--;
		}

		RTable_entry* rtable = malloc(sizeof(RTable_entry) * 100000);
		int rt_size = read_rtable(argv[1], rtable);

		RTable_entry *next = find_next_route(rtable, rt_size, ntohl(ip_hdr->daddr));

		next != NULL ? print_ip(next->prefix) : printf("Next null\n");
		free(rtable);
	}
}

