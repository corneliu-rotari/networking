# Router

`Author : Rotari Corneliu` `Group : 323CD`

Implemented the dataplane component of a router using C to work with ARP, ICMP, IP, and Ethernet networking protocols.
This involved implementing the routing rules specified in a static routing table and ensuring the router could forward packets to computers or other directly connected routers.

___

## Table of contents

- [Initialization](#initialization)
- [LPM](#longest-prefix-match)
- [ARP](#arp)
    - [Replay](#arp_replay)
    - [Request](#arp_request)
- [IP](#ip-icmp)

___

## Initialization

The router parses the static routing table and creates the `Arp_table` in which it stores the cached MAC address found by sending the Arp Request.
Here we create the queue to enqueue every packet we need to resend after an `ARP_Request`.

___

## Longest Prefix Match

It removes invalid entryies by checking if the prefix is equal to the prefix plus a bitwise and with the mask.
The table is sorted in ascending order by prefix and after that by mask.
The search in the static routing table is implemented with a binary-search.

___

## ARP

### ARP_Replay

It checks if the arp packet is a request and only then it replays, by calling the function `arp_replay()` which changes the infomration in the packet.
It puts the address of the host and the target.
It sends it back through the same interface.

### ARP_Request

It is only called if the router does not have an entry of the next hop MAC address.
It creates the ARP packet and waits for a response.
If any other packets are received before the ARP Replay, it enqueues them all to be handled after.

___

## IP (ICMP)

If the Ethernet type is IPV4_TYPE, the function processes the IP packet.
It checks the MAC and checksum, TTL, and if it's a personal ping reply or not.
If it's not, the function finds the next route, recalculates the checksum, and writes the address to the frame.
If the ARP entry is not found in the cache, the function sends an ARP request.
___
