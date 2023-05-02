#ifndef __TCP_CLIENT_SERVER
#define __TCP_CLIENT_SERVER 1

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

bool isExit(char *input);
bool isSubscribe(char *input);
bool isUnsubscribe(char *input);
const char *convert_type(uint8_t type);

struct pollfd *add_to_poll(struct pollfd *poll_fds, int fd, int *nr_fds);
struct pollfd *init_poll(int *nr_fds);
struct pollfd *remove_poll(struct pollfd *poll_fds, int fd, int *nr_fds, int pos);
void destory_poll(struct pollfd *poll_fds, int nr_fds);

#define DIE(assertion, call_description)                 \
  do                                                     \
  {                                                      \
    if (assertion)                                       \
    {                                                    \
      fprintf(stderr, "(%s, %d): ", __FILE__, __LINE__); \
      perror(call_description);                          \
      exit(EXIT_FAILURE);                                \
    }                                                    \
  } while (0)

#define MAX_LEN_BUFF 1500

struct tcp_app_com
{
  char topic[50];
  uint8_t type;
  union
  {
    struct {
      char id[10];
      uint8_t type_action;
      uint8_t sf;
    } req;
    struct {
      uint16_t port_udp;
      struct in_addr ip_udp;
      uint8_t type;
      char messege[MAX_LEN_BUFF];
    } rep;
  } un;
} __attribute__((packed));


struct message_info {
  uint8_t type;
  char payload[1500];
};

struct udp_packet
{
  char topic[50];
  struct message_info content;
} __attribute__((packed));

typedef struct udp_packet source_packet;
typedef struct tcp_app_com news_packet;

#define NEWS_REQ 1
#define NEWS_REP 0
#define NEWS_ACK 2

#define NEWS_SUB 1
#define NEWS_UNSUB 0

#endif
