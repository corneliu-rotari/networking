#ifndef _NEWSP_H
#define _NEWSP_H

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

#include <poll.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <sys/poll.h>
#include <sys/types.h>
#include <sys/socket.h>

#define NEWS_PACK_ID 0
#define NEWS_PACK_REQ 1
#define NEWS_PACK_REP 2
#define NEWS_PACK_ACK 3

#define NEWS_TYPE_SUB 1
#define NEWS_TYPE_UNSUB 0

struct udp_packet
{
  char topic[50];
  uint8_t type;
  char payload[1500];
} __attribute__((packed));


struct tcp_app_com
{
    uint8_t packet_type;
    uint16_t size;

    union
    {
        struct
        {
            char topic[50];
            uint8_t type_action;
            uint8_t sf;
        } req;

        struct
        {
            uint16_t port_udp;
            struct in_addr ip_udp;
            struct udp_packet content;
        } rep;

        char id[10];
    } un;

} __attribute__((packed));

typedef struct udp_packet source_packet;
typedef struct tcp_app_com news_packet;

int recv_tcp_packet(int fd, char* buffer_addr, int expcted_size);
int send_tcp_packet(int fd, char* buffer_addr, int size_to_sent);

#define NEWS_PACKET_HEADER_SIZE (sizeof(uint8_t) + sizeof(uint16_t))

#endif
