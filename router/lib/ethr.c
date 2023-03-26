#include "router.h"

void ethr_swap_mac_addr(Eth_hdr *eth_hdr)
{
	u_int8_t tmp[6];
	size_t len = sizeof(tmp);
	memcpy(&tmp, &eth_hdr->ether_dhost, len);
	memcpy(&eth_hdr->ether_dhost, &eth_hdr->ether_shost, len);
	memcpy(&eth_hdr->ether_shost, &tmp, len);
}
