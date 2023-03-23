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
	u_int8_t tmp[6];
	size_t len = sizeof(tmp);
	memcpy(&tmp, &eth_hdr->ether_dhost, len);
	memcpy(&eth_hdr->ether_dhost, &eth_hdr->ether_shost, len);
	memcpy(&eth_hdr->ether_shost, &tmp, len);
}



int main(int argc, char *argv[])
{
	char buf[MAX_PACKET_LEN];

	// Do not modify this line
	init(argc - 2, argv + 2);

	// Parse Routing table
	RTable_entry *rtable = malloc(sizeof(RTable_entry) * 100000);
	int rt_size = read_rtable(argv[1], rtable);
	int rt_size_new = remove_invalid_entries(rtable, rt_size);
	qsort(rtable, rt_size, sizeof(RTable_entry), compare_rtables_entry);
	rt_size = rt_size_new;

	// Intilizing ARP utilities
	queue q = queue_create();
	ARP_table *arp_table = malloc(sizeof(ARP_table));
	arp_table->cache = malloc(sizeof(ARP_entry) * ARP_CACHE_SIZE);
	arp_table->size = 0;
	// arp_table->size= parse_arp_table("s_arp_table.txt", arp_table->cache);
	arp_table->capacity = ARP_CACHE_SIZE;

	while (1)
	{
		int interface;
		size_t len;



		if (!queue_empty(q)) {
			queue_enq(q, (void *) create_packet(buf, len, interface));

			Packet* old_packet = (Packet *) (queue_deq(q));
			memcpy(buf, old_packet->buf, MAX_PACKET_LEN);
			interface = old_packet->interface;
			len = old_packet->len;
			free(old_packet);

		} else {
			interface = recv_from_any_link(buf, &len);
			DIE(interface < 0, "recv_from_any_links");
			printf("\nRecived a packet\n");
		}

		Eth_hdr *eth_hdr = (Eth_hdr *)buf;
		uint16_t eth_type = ntohs(eth_hdr->ether_type);

		switch (eth_type) {
			case ARP_TYPE: {
				// ARP PROTOCOL REPALY
				ARP_hdr *arp_hdr = (ARP_hdr *)(buf + sizeof(Eth_hdr));
				if (ntohs(arp_hdr->op) == 1) {
					printf("ARP SEARCH and REPLAY\n");

					arp_replay(eth_hdr, arp_hdr, interface);
					send_to_link(interface, buf, len);
				}
				break;
			}
			case IPV4_TYPE: {

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
					icmp_change(icmp_hdr, ICMP_TIME_EXCEED, len - sizeof(Eth_hdr) - sizeof(IP_hdr) + add_to_size);
					swap_mac_addr(eth_hdr);
					send_to_link(interface, buf, len + add_to_size);
					continue;
				}
				else
				{
					ip_hdr->ttl--;
				}
				
				uint32_t interface_ip = convert_ip_uint32(get_interface_ip(interface));

				if (interface_ip == ip_hdr->daddr) {
					icmp_change(icmp_hdr, ICMP_REPLAY, len - sizeof(Eth_hdr) - sizeof(IP_hdr));
					swap_mac_addr(eth_hdr);
					send_to_link(interface, buf, len);
					continue;
				}

				// Find next route
				RTable_entry *next = find_next_route(rtable, rt_size, ntohl(ip_hdr->daddr));

				if (next == NULL)
				{
					// ICMP Destination Unreachble
					ip_hdr->check = htons(old_check);
					memcpy(((char*) icmp_hdr) + sizeof(icmp_hdr), ip_hdr, sizeof(IP_hdr) + 8);

					uint16_t add_to_size = (uint16_t) (sizeof(ICMP_hdr) + sizeof(IP_hdr) + 8);
					ip_hdr->tot_len += htons(add_to_size);
					ip_hdr->protocol = 1;
					icmp_change(icmp_hdr, ICMP_DEST_UNREACH, len - sizeof(Eth_hdr) - sizeof(IP_hdr) + add_to_size);
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
				ARP_entry* arp_entry = arp_cache_search(arp_table, next);
				if(arp_entry == NULL) {
					printf("ARP REQUEST\n");
					arp_request(buf, next, q, &len, &interface, arp_table);
					get_interface_mac(next->interface, eth_hdr->ether_shost);
				} else {
					get_interface_mac(next->interface, eth_hdr->ether_shost);
					memcpy(eth_hdr->ether_dhost, arp_entry->mac, sizeof(arp_entry->mac));
				}
				
				send_to_link(interface, buf, len);
			} default:
				break;
		}
	}
	free(q);
	free(arp_table);
	free(rtable);
}
