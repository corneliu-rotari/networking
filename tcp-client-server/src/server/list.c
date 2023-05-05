#include "server.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

struct node *init_node(news_packet *to_store)
{
    struct node *newNode = malloc(sizeof(struct node));
    memcpy(&newNode->messege, to_store, sizeof(news_packet));
    newNode->next = NULL;
    return newNode;
}


void destory_list(struct topics_clients *cli_info)
{
    if (cli_info == NULL || cli_info->next_mess == NULL)
        return;
    
    struct node *newNode = cli_info->next_mess->head;
    struct node *aux;

    while (newNode != NULL)
    {
        aux = newNode;
        newNode = newNode->next;
        free(aux);
    }
    free(cli_info->next_mess);
    cli_info->next_mess = NULL;
}

void store_packet(struct topics_clients *cli_info, news_packet *to_store)
{
    if (cli_info->next_mess == NULL)
    {
        cli_info->next_mess = malloc(sizeof(struct list));
        cli_info->next_mess->head = init_node(to_store);
        cli_info->next_mess->tail = cli_info->next_mess->head;
    }
    else
    {
        struct node *newNode = init_node(to_store);
        cli_info->next_mess->tail->next = newNode;
        cli_info->next_mess->tail = newNode;
    }
}

void forward_packet(struct topics_clients *cli_info, int fd)
{
    struct node *newNode = cli_info->next_mess->head;
    while (newNode != NULL)
    {
        send_tcp_packet(fd, (char *)&newNode->messege, 
            NEWS_PACKET_HEADER_SIZE + ntohs(newNode->messege.size));
        newNode = newNode->next;
    }
    destory_list(cli_info);
}
