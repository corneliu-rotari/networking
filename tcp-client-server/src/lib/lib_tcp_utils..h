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

typedef struct CSP
{
  char id[MAX_LEN_BUFF];
} CSP_packet;

typedef struct udp_packet
{

}

#endif
