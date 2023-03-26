#include "router.h"

void print_ip(uint32_t ip)
{
	for (int i = 0; i < IP_LEN; i++)
	{
		printf("%hhu.", *((uint8_t *)(&ip) + sizeof(uint8_t) * i));
	}
	printf("   %u\n", ip);
}

void print_mac(uint8_t *mac)
{
	for (int i = 0; i < ETHER_MAC_LEN; i++)
	{
		printf("%x:", mac[i]);
	}
	printf("\n");
}
