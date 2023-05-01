#ifndef _SERVER_H
#define _SERVER_H
#include <stdbool.h>

struct topic;

typedef struct client_info
{
    char client_id[10];
    struct topic* subscribed_to;
} client_info;

typedef struct topic
{
    client_info* subscribers;
} topic;

bool add_client(client_info **list_addr, char id[10], int nr_clients);
bool remove_client(client_info **list_addr, int pos, int nr_cli);

#endif
