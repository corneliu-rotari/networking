#ifndef __TCP_CLIENT_SERVER
#define __TCP_CLIENT_SERVER 1

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

bool isExit(char *input);
bool isSubscribe(char *input);
bool isUnsubscribe(char *input);

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
  char id[10];
  char topic[50];
  uint8_t flags;

} __attribute__((packed));

struct udp_packet
{
  char topic[50];
  uint8_t type;
  char payload[1500];
} __attribute__((packed));

typedef struct udp_packet source_packet;
typedef struct tcp_app_com news_packet;







#endif
