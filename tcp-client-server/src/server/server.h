#ifndef _SERVER_H
#define _SERVER_H
#include "../lib/lib_tcp_utils.h"

struct node {
    news_packet messege;
    struct node* next;
};

struct list {
    struct node* head;
    struct node* tail;
};

typedef struct topic
{
    char topic[51];
    uint8_t sf;
    struct list* message;
} topic;

typedef struct client_info
{
    char client_id[10];
    uint8_t active;
    int fd;
    struct topic* subscribed_to;
    int nr_subscribed;
} client_info;

typedef struct client_database {
    client_info* clients_information;
    int nr_clients;
} client_database;


bool connect_client(client_database* list, char id[10], int fd);
bool disconnect_client(client_database *list, int fd);
bool add_topic_to_client(client_database *list, int fd, news_packet *info_addr);
struct topic *search_topic(struct topic *list, char name[50], int nr_topics);

#endif
