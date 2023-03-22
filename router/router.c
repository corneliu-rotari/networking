#include "router.h"

struct packet {
	char buf[MAX_PACKET_LEN];
	int len;
	int interface;
};

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

uint32_t convert_ip_uint32(char* sir) {
	printf("Before conversion : %s\n", sir);
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
			// if (add == 3)
			// 	break;
		}
	}
	printf("After :");
	return to_ret;
}

int main(int argc, char *argv[])
{
	char buf[MAX_PACKET_LEN];

	// Do not modify this line
	init(argc - 2, argv + 2);
	queue q = queue_create();

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
		printf("\nRecived a packet\n");
		
		// struct packet *arrived = malloc(sizeof(struct packet));
		// memcpy(arrived->buf, buf, MAX_PACKET_LEN);
		// arrived->interface = interface;
		// arrived->len = len;
		// queue_enq(q, (void *) arrived);


		Eth_hdr *eth_hdr = (Eth_hdr *)buf;
		uint16_t eth_type = ntohs(eth_hdr->ether_type);

		// if (eth_type == ARP_TYPE)
		// {
		// 	// ARP PROTOCOL REPALY
		// 	printf("ARP SEARCH and REPLAY\n");
		// 	ARP_hdr *arp_hdr = (ARP_hdr *)(buf + sizeof(Eth_hdr));

		// 	arp_replay(eth_hdr, arp_hdr, interface);
		// }
		// else 
		if (eth_type == IPV4_TYPE)
		{

			// IPv4 PROTOCOL
			printf("IP Control\n");

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
				ip_hdr->check = htons(old_check);
				memcpy(((char*) icmp_hdr) + sizeof(icmp_hdr), ip_hdr, sizeof(IP_hdr) + 8);

				uint16_t add_to_size = (uint16_t) (sizeof(ICMP_hdr) + sizeof(IP_hdr) + 8);
				ip_hdr->tot_len += htons(add_to_size);
				ip_hdr->protocol = 1;
				change_icmp(icmp_hdr, ICMP_TIME_EXCEED, len - sizeof(Eth_hdr) - sizeof(IP_hdr) + add_to_size);
				swap_mac_addr(eth_hdr);
				send_to_link(interface, buf, len + add_to_size);
				continue;
			}
			else
			{
				ip_hdr->ttl--;
			}
			
			uint32_t interface_ip = convert_ip_uint32(get_interface_ip(interface));
			print_ip(interface_ip);
			print_ip(ip_hdr->daddr);
			if (interface_ip == ip_hdr->daddr) {
				print_ip(interface_ip);
				change_icmp(icmp_hdr, ICMP_REPLAY, len - sizeof(Eth_hdr) - sizeof(IP_hdr));
				swap_mac_addr(eth_hdr);
				send_to_link(interface, buf, len);
				continue;
			}

			// Find next route
			RTable_entry *next = find_next_route(rtable, rt_size, ntohl(ip_hdr->daddr));

			if (next == NULL)
			{
				printf("Next null\n");
				// TODO send back ICMP Destination Unreachble
				ip_hdr->check = htons(old_check);
				memcpy(((char*) icmp_hdr) + sizeof(icmp_hdr), ip_hdr, sizeof(IP_hdr) + 8);

				uint16_t add_to_size = (uint16_t) (sizeof(ICMP_hdr) + sizeof(IP_hdr) + 8);
				ip_hdr->tot_len += htons(add_to_size);
				ip_hdr->protocol = 1;
				change_icmp(icmp_hdr, ICMP_DEST_UNREACH, len - sizeof(Eth_hdr) - sizeof(IP_hdr) + add_to_size);
				swap_mac_addr(eth_hdr);
				send_to_link(interface, buf, len + add_to_size);
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
