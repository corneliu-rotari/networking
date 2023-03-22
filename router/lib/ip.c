#include "router.h"

int ip_hdr_check(Eth_hdr *eth_hdr, IP_hdr *ip_hdr, int interface, uint16_t old_check)
{
	// Check MAC
	uint8_t *router_mac_addr = malloc(sizeof(eth_hdr->ether_dhost));
	uint8_t *eth_dest_toh = malloc(sizeof(eth_hdr->ether_dhost));
	memcpy(eth_dest_toh, eth_hdr->ether_dhost, sizeof(eth_hdr->ether_dhost));

	get_interface_mac(interface, router_mac_addr);

	for (int i = 0; i < sizeof(eth_hdr->ether_dhost); i++)
	{
		if (eth_dest_toh[i] != router_mac_addr[i])
		{
			free(router_mac_addr);
			free(eth_dest_toh);
			return 0;
		}
	}
	free(router_mac_addr);
	free(eth_dest_toh);

	// Check checksum
	ip_hdr->check = 0;
	uint16_t check = checksum((uint16_t *)ip_hdr, sizeof(IP_hdr));

	if (check != old_check)
	{
		return 0;
	}
	return 1;
}

int compare_rtables_entry(const void *a, const void *b)
{
	RTable_entry a_entry = *(RTable_entry *)a;
	RTable_entry b_entry = *(RTable_entry *)b;
	uint32_t a_en = ntohl(a_entry.prefix);
	uint32_t b_en = ntohl(b_entry.prefix);
	uint32_t a_mask = ntohl(a_entry.mask);
	uint32_t b_mask = ntohl(b_entry.mask);

	if (a_en != b_en)
	{
		return a_en - b_en;
	}
	else
	{
		return a_mask - b_mask;
	}
}

RTable_entry *find_next_route(RTable_entry *table, int len, uint32_t ip_daddr)
{

	int pos = 0, first = 0, last = len;
	RTable_entry *next_route = NULL;
	uint32_t max_mask = 0;

	while (last - first > 1)
	{
		pos = (first + last) / 2;
		uint32_t prefix = ntohl(table[pos].prefix);
		uint32_t mask = ntohl(table[pos].mask);
		uint32_t searched_ip = (ip_daddr & PREFIX_MASK);
		// print_ip(table[pos].prefix);

		if (searched_ip == prefix)
		{
			// printf("\n\nFound Prefix : ");print_ip(prefix);
			// printf("IP + mask : ");print_ip(searched_ip);

			while (searched_ip == table[pos + 1].prefix)
			{
				searched_ip = (ip_daddr & mask);
				pos++;
			}
			if (mask > max_mask)
			{
				next_route = &table[pos];
				max_mask = mask;
			}
			first = pos;
			continue;
		}
		else if (searched_ip > prefix)
		{
			first = pos + 1;
		}
		else
		{
			last = pos;
		}
	}

	return next_route;
}

void change_icmp(ICMP_hdr *icmp_hdr, uint8_t type, int len)
{
	icmp_hdr->type = type;
	icmp_hdr->code = 0;
	icmp_hdr->checksum = 0;
	icmp_hdr->checksum = htons(checksum((uint16_t *)icmp_hdr, len));
}
