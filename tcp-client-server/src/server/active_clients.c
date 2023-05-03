#include "server.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

bool connect_client(client_database *list, char id[10], int fd)
{

    if (!list->clients_information)
    {
        list->clients_information = malloc(sizeof(client_info));
    }
    else
    {
        for (int i = 0; i < list->nr_clients; i++)
        {
            size_t len_id = strlen(id);

            if (strncmp(list->clients_information[i].client_id, id, len_id) == 0 &&
                len_id == strlen(list->clients_information[i].client_id))
            {
                if (list->clients_information[i].active == 1)
                {
                    return false;
                }
                else
                {
                    list->clients_information[i].active = 1;
                    list->clients_information[i].fd = fd;
                    return true;
                }
            }
        }
        list->clients_information = realloc(list->clients_information, sizeof(client_info) * (list->nr_clients + 1));
    }

    strncpy(list->clients_information[list->nr_clients].client_id, id, 10);
    list->clients_information[list->nr_clients].subscribed_to = NULL;
    list->clients_information[list->nr_clients].nr_subscribed = 0;
    list->clients_information[list->nr_clients].active = 1;
    list->clients_information[list->nr_clients].fd = fd;
    list->nr_clients++;
    return true;
}

client_info *search_client(client_database *list, int fd)
{
    for (int i = 0; i < list->nr_clients; i++)
    {
        if (list->clients_information[i].fd == fd)
        {
            return &list->clients_information[i];
        }
    }
    return NULL;
}

bool disconnect_client(client_database *list, int fd)
{
    client_info *client_addr = search_client(list, fd);
    if (!client_addr)
        return false;

    printf("Client %s disconnected.\n", client_addr->client_id);
    client_addr->active = 0;
    client_addr->fd = 0;
    return true;
}

struct topic *search_topic(struct topic *list, char name[50], int nr_topics)
{
    for (int i = 0; i < nr_topics; i++)
    {
        if (strncmp(list[i].topic, name, strlen(name)) == 0 &&
            strlen(name) == strlen(list->topic))
        {
            return &list[i];
        }
    }
    return NULL;
}

bool add_topic_to_client(client_database *list, int fd, news_packet *info_addr)
{
    client_info *client_addr = search_client(list, fd);
    if (!client_addr)
        return false;

    if (client_addr->nr_subscribed == 0)
    {
        client_addr->subscribed_to = malloc(sizeof(struct topic));
    }
    else
    {
        char topic[51] = {'\0'};
        memcpy(topic, info_addr->topic, sizeof(50));
        // struct topic *exists = search_topic(client_addr->subscribed_to, topic, client_addr->nr_subscribed);
        
        // // TODO : Add sf

        // if (exists)
        //     return false;

        client_addr->subscribed_to = realloc(client_addr->subscribed_to, sizeof(struct topic) * (client_addr->nr_subscribed + 1));
    }

    memcpy(client_addr->subscribed_to[client_addr->nr_subscribed].topic, info_addr->topic, 50);
    client_addr->subscribed_to[client_addr->nr_subscribed].sf = info_addr->un.req.sf;
    client_addr->subscribed_to[client_addr->nr_subscribed].message = NULL;
    client_addr->nr_subscribed++;
    return true;
}
