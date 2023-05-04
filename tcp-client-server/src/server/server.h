#ifndef _SERVER_H
#define _SERVER_H
#include "../lib/lib_tcp_utils.h"
#include <netinet/tcp.h>

struct node
{
    news_packet messege;
    struct node *next;
};

struct list
{
    struct node *head;
    struct node *tail;
};

typedef struct client_info
{
    char client_id[10];
    uint8_t active;
    int fd;
} client_info;

struct topics_clients
{
    int pos_in_client_vector;
    uint8_t sf;
    struct list *next_mess;
};

typedef struct topic
{
    char topic[51];
    struct topics_clients* subscribers;
    int nr_subscribers;
} topic;

typedef struct client_database
{
    client_info *clients_information;
    int nr_clients;
    struct topic *exsitent_topics;
    int nr_topics;
} client_database;

void connect_tcp_client_to_server(int tcp_socket, client_database *c_db, int *nr_fds_addr, struct pollfd **poll_fds_addr);
void parse_and_exec_client_request(client_database *c_db, news_packet * recv_packet_addr, int clinet_fd);

bool add_client_to_client_list(client_database *c_db, char id[10], int fd);
client_info *search_client(client_database *c_db, int fd, int* pos);
bool disconnect_client(client_database *c_db, int fd);


void create_topic(client_database *c_db, news_packet *info_addr);
bool add_client_to_topic(struct topic *topic_addr, int pos, news_packet *info_addr);
struct topic *search_topic(client_database *c_db, char name[50]);
void remove_client_from_topic(struct topic *topic_addr, int pos);

void store_packet(struct topics_clients *cli_info, news_packet *to_store);
void forward_packet(struct topics_clients *cli_info, int fd);
void destory_list(struct topics_clients *cli_info);
#endif
