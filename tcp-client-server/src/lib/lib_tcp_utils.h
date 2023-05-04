#ifndef __TCP_CLIENT_SERVER
#define __TCP_CLIENT_SERVER 1

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>

#include <poll.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <sys/poll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "news_protocol.h"

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


bool isExit(char *input);
bool isSubscribe(char *input);
bool isUnsubscribe(char *input);
const char *convert_type(uint8_t type);

struct pollfd *add_to_poll(struct pollfd *poll_fds, int fd, int *nr_fds);
struct pollfd *init_poll(int *nr_fds);
struct pollfd *remove_poll(struct pollfd *poll_fds, int fd, int *nr_fds, int pos);
void destory_poll(struct pollfd *poll_fds, int nr_fds);
#endif
