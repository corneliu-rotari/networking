#include "router.h"

void print_ip(uint32_t ip)
{
	for (int i = 0; i < 4; i++)
	{
		printf("%hhu.", *((uint8_t *)(&ip) + sizeof(uint8_t) * i));
	}
	printf("   %u\n", ip);
}

void print_mac(uint8_t *mac)
{
	for (int i = 0; i < 6; i++)
	{
		printf("%x:", mac[i]);
	}
	printf("\n");
}

void swap_mac_addr(Eth_hdr *eth_hdr)
{
	printf("Dest :");
	print_mac((eth_hdr->ether_dhost));
	printf("Source :");
	print_mac((eth_hdr->ether_shost));
	u_int8_t tmp[6];
	size_t len = sizeof(tmp);
	memcpy(&tmp, &eth_hdr->ether_dhost, len);
	memcpy(&eth_hdr->ether_dhost, &eth_hdr->ether_shost, len);
	memcpy(&eth_hdr->ether_shost, &tmp, len);
	printf("Dest :");
	print_mac((eth_hdr->ether_dhost));
	printf("Source :");
	print_mac((eth_hdr->ether_shost));
}

int main(int argc, char *argv[])
{
	char buf[MAX_PACKET_LEN];

	// Do not modify this line
	init(argc - 2, argv + 2);

	RTable_entry *rtable = malloc(sizeof(RTable_entry) * 100000);
	int rt_size = read_rtable(argv[1], rtable);
	qsort(rtable, rt_size, sizeof(RTable_entry), compare_rtables_entry);

	ARP_entry *arp_table = malloc(sizeof(ARP_entry) * ARP_CACHE_SIZE);
	int arpt_size = parse_arp_table("arp_table.txt", arp_table);

	while (1)
	{

		int interface;
		size_t len;

		interface = recv_from_any_link(buf, &len);
		DIE(interface < 0, "recv_from_any_links");
		printf("Recived a packet\n");

		Eth_hdr *eth_hdr = (Eth_hdr *)buf;
		uint16_t eth_type = ntohs(eth_hdr->ether_type);

		if (eth_type == ARP_TYPE)
		{

			// ARP PROTOCOL REPALY
			printf("\nARP SEARCH and REPLAY\n");
			ARP_hdr *arp_hdr = (ARP_hdr *)(buf + sizeof(Eth_hdr));

			arp_replay(eth_hdr, arp_hdr, interface);
		}
		else if (eth_type == IPV4_TYPE)
		{

			// IPv4 PROTOCOL
			printf("\nIP Control\n");

			IP_hdr *ip_hdr = (IP_hdr *)(buf + sizeof(Eth_hdr));
			ICMP_hdr *icmp_hdr = (ICMP_hdr *)(buf + sizeof(Eth_hdr) + sizeof(IP_hdr));
			uint16_t old_check = ntohs(ip_hdr->check);

			// Check MAC and Cheksum
			if (!ip_hdr_check(eth_hdr, ip_hdr, interface, old_check))
			{
				printf("Not for me mac or check sum\n");
				continue;
			}

			// Check ttl
			if (ip_hdr->ttl <= 1)
			{
				printf("TTL Exipred\n");
				change_icmp(icmp_hdr, TIME_EXCEED, len - sizeof(Eth_hdr) - sizeof(IP_hdr));
				swap_mac_addr(eth_hdr);
				send_to_link(interface, buf, len);
				continue;
			}
			else
			{
				ip_hdr->ttl--;
			}

			// Find next route
			RTable_entry *next = find_next_route(rtable, rt_size, ntohl(ip_hdr->daddr));

			if (next == NULL)
			{
				printf("Next null\n");
				// TODO send back ICMP Destination Unreachble
				change_icmp(icmp_hdr, DEST_UNREACH, len - sizeof(Eth_hdr) - sizeof(IP_hdr));
				swap_mac_addr(eth_hdr);
				send_to_link(interface, buf, len);
				continue;
			}
			else
			{
				print_ip(next->prefix);
				print_ip(next->next_hop);
				print_ip(next->mask);
			}
			interface = next->interface;

			// Recalulate checksum
			ip_hdr->check = htons(checksum((uint16_t *)ip_hdr, sizeof(IP_hdr)));

			// Write address to frame
			for (int i = 0; i < arpt_size; i++)
			{
				if (arp_table[i].ip == next->next_hop)
				{
					// printf("Writing addess to eth header\n");
					get_interface_mac(next->interface, eth_hdr->ether_shost);
					memcpy(&eth_hdr->ether_dhost, &arp_table[i].mac, sizeof(arp_table[i].mac));
					break;
				}
			}
			// arp_request(eth_hdr, next);
		}
		// printf("Dest :"); print_mac((eth_hdr->ether_dhost));
		// printf("Source :"); print_mac((eth_hdr->ether_shost));
		// Send to link
		send_to_link(interface, buf, len);
	}

	free(rtable);
}
