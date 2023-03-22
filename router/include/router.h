#include "protocols.h"
#include "lib.h"
#include "queue.h"
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>

typedef struct ether_header Eth_hdr;
typedef struct iphdr IP_hdr;
typedef struct arp_header ARP_hdr;
typedef struct icmphdr ICMP_hdr;

typedef struct route_table_entry RTable_entry;
typedef struct arp_entry ARP_entry;

#define ARP_TYPE 0x0806
#define IPV4_TYPE 0x0800
#define PREFIX_MASK 4294967040
#define ICMP_DEST_UNREACH 3
#define ICMP_TIME_EXCEED 11
#define ICMP_REPLAY 0
#define ARP_CACHE_SIZE 12

RTable_entry *find_next_route(RTable_entry *table, int len, uint32_t ip_daddr);
int compare_rtables_entry(const void *a, const void *b);

int ip_hdr_check(Eth_hdr *eth_hdr, IP_hdr *ip_hdr, int interface, uint16_t old_check);

void change_icmp(ICMP_hdr *icmp_hdr, uint8_t type, int len);

int arp_replay(Eth_hdr *eth_hdr, ARP_hdr *arp_hdr, int interface);
int arp_request(Eth_hdr *eth_hdr, RTable_entry *next);
