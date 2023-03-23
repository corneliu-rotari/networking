#include "router.h"

int arp_replay(Eth_hdr *eth_hdr, ARP_hdr *arp_hdr, int interface)
{
	// char* ip = get_interface_ip(interface);
	// printf("%s\n", ip);
	// print_ip(arp_hdr->tpa);
	memcpy(arp_hdr->tha, arp_hdr->sha, arp_hdr->hlen);
	get_interface_mac(interface, arp_hdr->sha);

	arp_hdr->op = htons(2);

	uint32_t adr_tmp = arp_hdr->tpa;
	arp_hdr->tpa = arp_hdr->spa;
	arp_hdr->spa = adr_tmp;

	memcpy(eth_hdr->ether_dhost, arp_hdr->tha, arp_hdr->hlen);
	memcpy(eth_hdr->ether_shost, arp_hdr->sha, arp_hdr->hlen);
	return 1;
}

Packet* create_packet(char *buf, int len, int interface) {
	Packet *arrived = malloc(sizeof(struct packet));
	memcpy(arrived->buf, buf, MAX_PACKET_LEN);
	arrived->interface = interface;
	arrived->len = len;
	return arrived;
}

ARP_entry* arp_cache_search(ARP_table *arp_table, RTable_entry *next) {
	for (int i = 0; i < arp_table->size; i++)
	{
		if (arp_table->cache[i].ip == next->next_hop)
		{
			return &arp_table->cache[i];
		}
	}
	return NULL;
}

int arp_request(char* buf, RTable_entry *next, queue q, size_t *len_add,
				int* interface_add, ARP_table* arp_table)
{
	Eth_hdr *eth_hdr = (Eth_hdr *)(buf);
	int interface = *interface_add;
	size_t len = *len_add;

// Save old packet
	queue_enq(q, (void *) create_packet(buf, len, interface));

// Generate ARP REQUEST
	eth_hdr->ether_type = htons(ARP_TYPE);
	get_interface_mac(interface, eth_hdr->ether_shost);
	for (int i = 0; i < 6; i++)
	{
		eth_hdr->ether_dhost[i] = 255;
	}
	ARP_hdr* arp_hdr = (ARP_hdr*)(buf + sizeof(Eth_hdr));
	arp_hdr->htype = htons(1);
	arp_hdr->ptype = htons(IPV4_TYPE);
	arp_hdr->hlen = 6;
	arp_hdr->plen = 4;
	arp_hdr->op = htons(1);
	memcpy(arp_hdr->sha, eth_hdr->ether_shost, sizeof(arp_hdr->sha));
	for (int i = 0; i < 6; i++)
	{
		arp_hdr->tha[i] = 0;
	}
	arp_hdr->tpa = next->next_hop;
	arp_hdr->spa = convert_ip_uint32(get_interface_ip(next->interface));
	
// Send request
	interface = next->interface;
	len = sizeof(Eth_hdr) + sizeof(ARP_hdr);
	send_to_link(interface, buf, len);

// Receive replay
	interface = recv_from_any_link(buf, &len);
	DIE(interface < 0, "recv_from_any_links");

	printf("-------here %hu\n", htons(arp_hdr->op));
	
	while (ntohs(eth_hdr->ether_type) != ARP_TYPE || arp_hdr->op != htons(2))
	{
		printf("Type %hu %hu", ntohs(eth_hdr->ether_type), ARP_TYPE);
		printf("Type %hu %hu", arp_hdr->op, htons(2));
		queue_enq(q, (void *) create_packet(buf, len, interface));
		interface = recv_from_any_link(buf, &len);
		DIE(interface < 0, "recv_from_any_links");
	}

// Add to cache
	if (arp_table->size == arp_table->capacity) {
		arp_table->capacity *= 2;
		arp_table->cache = realloc(arp_table->cache, sizeof(ARP_entry) * arp_table->capacity);
	}
	arp_table->cache[arp_table->size].ip = arp_hdr->spa;
	memcpy(arp_table->cache[arp_table->size].mac, arp_hdr->sha, arp_hdr->hlen);
	arp_table->size++;
	printf("S_IP : "); print_ip( arp_hdr->spa);
	printf("S_Mac : "); print_mac(arp_hdr->sha);

// Resotre the last packet
	if (!queue_empty(q)) {
		Packet* old_packet = (Packet *) (queue_deq(q));
		memcpy(buf, old_packet->buf, MAX_PACKET_LEN);
		*interface_add = old_packet->interface;
		*len_add = old_packet->len;
		free(old_packet);
	}

// Add new information
	get_interface_mac(next->interface, eth_hdr->ether_shost);
	memcpy(eth_hdr->ether_dhost, &arp_table->cache[arp_table->size - 1].mac, sizeof(arp_table->cache[arp_table->size].mac));

	return 1;
}
