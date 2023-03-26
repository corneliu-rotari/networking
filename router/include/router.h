#ifndef ROUTER_H
#define ROUTER_H

#include "protocols.h"
#include "lib.h"
#include "queue.h"
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>

/* Incapsulate a packet information and the sender
 */
typedef struct packet
{
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

/* Incapsulates the information abaout the ARP */
#define ARP_TYPE 0x0806
#define ARP_CACHE_SIZE 12
#define ARP_REQUEST 1
#define ARP_REPLAY 2

typedef struct arp_table
{
    ARP_entry *cache;
    int size;
    int capacity;
} ARP_table;

/* Modifies the arp header to be sent as arp replay */
int arp_replay(Eth_hdr *eth_hdr, ARP_hdr *arp_hdr, int interface);

/* Creates an arp request to find the phisical address of the next route and adds it to the cache */
int arp_request(char *buf, RTable_entry *next, queue q, size_t *len_add, int *interface_add, ARP_table *arp_table);

/* Searchs through the cache to find if the hosts knows the mac of the next ip */
ARP_entry *arp_cache_search(ARP_table *arp_table, RTable_entry *next);

/* Incapsulates the information abaout the IPv4 */
#define IPV4_TYPE 0x0800
#define IP_MAX 4294967295
#define IP_LEN 4

/* Checks the checksum and if the packet is for this host */
int ip_hdr_check(Eth_hdr *eth_hdr, IP_hdr *ip_hdr, int interface, uint16_t old_check);

/* Converts an ip from the char form to the uint32_t form */
uint32_t convert_ip_uint32(char *sir);

#define ETHER_TYPE 1
#define ETHER_MAC_LEN 6

/* Swaps the source and destions mac address */
void ethr_swap_mac_addr(Eth_hdr *eth_hdr);

#define ICMP_DEST_UNREACH 3
#define ICMP_TIME_EXCEED 11
#define ICMP_REPLAY 0

/* Binary searches through the routing table and finds the best route */
RTable_entry *find_next_route(RTable_entry *table, int len, uint32_t ip_daddr);

/* Utility function to sort entries ascending by the prefix and mask */
int compare_rtables_entry(const void *a, const void *b);

/* Changes the entrys that the prefix dont match with the prefix & mask */
int remove_invalid_entries(RTable_entry *table, int size);

/* Changes the icmp header to send a error packet back */
void icmp_change(ICMP_hdr *icmp_hdr, uint8_t type, int len);

/* Svaes infomration about a received packet into a structure */
Packet *create_packet(char *buf, int len, int interface);

/* Debug function */
void print_mac(uint8_t *mac);

/* Debug function */
void print_ip(uint32_t ip);

#endif
