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

int remove_invalid_entries(RTable_entry* table, int size) {
	int nr_removed = 0;
	for (int i = 0; i < size; i++)
	{
		uint32_t a_en = ntohl(table[i].prefix);
		uint32_t a_mask = ntohl(table[i].mask);
		if ((a_en & a_mask) != a_en) {
			table[i].prefix = MAX_IP;
			nr_removed++;
		}
	}
	return size - nr_removed;
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

	while (last - first >= 1)
	{
		pos = (first + last) / 2;
		uint32_t prefix = ntohl(table[pos].prefix);
		uint32_t mask = ntohl(table[pos].mask);
		uint32_t searched_ip = (ip_daddr & mask);

		if (searched_ip == prefix)
		{
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

			if (last - first == 1)
				break;
			else
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

void icmp_change(ICMP_hdr *icmp_hdr, uint8_t type, int len)
{
	icmp_hdr->type = type;
	icmp_hdr->code = 0;
	icmp_hdr->checksum = 0;
	icmp_hdr->checksum = htons(checksum((uint16_t *)icmp_hdr, len));
}

uint32_t convert_ip_uint32(char* sir) {
	int last_point = 0;
	uint32_t to_ret = 0;
	int add = 0;
	sir[strlen(sir)] = '.';
	for (int i = 0; i < strlen(sir); i++)
	{
		char buf[4] = { 0 };
		if (sir[i] == '.') {
			memcpy(buf, sir + last_point, i - last_point);
			uint8_t sub_part = (uint8_t)atoi(buf);
			memcpy(((uint8_t*) (&to_ret)) + add, &sub_part, sizeof(uint8_t));
			add++;
			last_point = i  + 1;
		}
	}
	return to_ret;
}
