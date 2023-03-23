#include "protocols.h"
#include "lib.h"
#include "queue.h"
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>

typedef struct packet {
	char buf[MAX_PACKET_LEN];
	int len;
	int interface;
} Packet;


typedef struct ether_header Eth_hdr;
typedef struct iphdr IP_hdr;
typedef struct arp_header ARP_hdr;
typedef struct icmphdr ICMP_hdr;
typedef struct route_table_entry RTable_entry;
typedef struct arp_entry ARP_entry;

typedef struct arp_table {
    ARP_entry* cache;
    int size;
    int capacity;
} ARP_table;


#define ARP_TYPE 0x0806
#define IPV4_TYPE 0x0800
#define MAX_IP 4294967295
#define ICMP_DEST_UNREACH 3
#define ICMP_TIME_EXCEED 11
#define ICMP_REPLAY 0
#define ARP_CACHE_SIZE 12

RTable_entry *find_next_route(RTable_entry *table, int len, uint32_t ip_daddr);
int compare_rtables_entry(const void *a, const void *b);
int remove_invalid_entries(RTable_entry* table, int size);

void icmp_change(ICMP_hdr *icmp_hdr, uint8_t type, int len);
int ip_hdr_check(Eth_hdr *eth_hdr, IP_hdr *ip_hdr, int interface, uint16_t old_check);
uint32_t convert_ip_uint32(char* sir);

int arp_replay(Eth_hdr *eth_hdr, ARP_hdr *arp_hdr, int interface);
int arp_request(char* buf, RTable_entry *next, queue q,size_t *len_add, int* interface_add, ARP_table* arp_table);
ARP_entry* arp_cache_search(ARP_table *arp_table, RTable_entry *next);
Packet* create_packet(char *buf, int len, int interface);

void print_mac(uint8_t *mac);
void print_ip(uint32_t ip);
