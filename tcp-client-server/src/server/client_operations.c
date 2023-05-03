#include "server.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

bool connect_client(client_database *c_db, char id[10], int fd)
{

    if (!c_db->clients_information)
    {
        c_db->clients_information = malloc(sizeof(client_info));
    }
    else
    {
        for (int i = 0; i < c_db->nr_clients; i++)
        {
            size_t len_id = strlen(id);

            if (strncmp(c_db->clients_information[i].client_id, id, 10) == 0 &&
                len_id == strlen(c_db->clients_information[i].client_id))
            {
                if (c_db->clients_information[i].active == 1)
                {
                    return false;
                }
                else
                {
                    c_db->clients_information[i].active = 1;
                    c_db->clients_information[i].fd = fd;
                    for (int j = 0; j < c_db->nr_topics; j++)
                    {
                        for (int k = 0; k < c_db->exsitent_topics[j].nr_subscribers; k++)
                        {
                            if (c_db->exsitent_topics[j].subscribers[k].pos_in_client_vector == i &&
                                c_db->exsitent_topics[j].subscribers[k].next_mess != NULL)
                            {
                                forward_packet(&c_db->exsitent_topics[j].subscribers[k], fd);
                            }
                        }
                    }

                    return true;
                }
            }
        }
        c_db->clients_information = realloc(c_db->clients_information, sizeof(client_info) * (c_db->nr_clients + 1));
    }

    strncpy(c_db->clients_information[c_db->nr_clients].client_id, id, 10);
    c_db->clients_information[c_db->nr_clients].active = 1;
    c_db->clients_information[c_db->nr_clients].fd = fd;
    c_db->nr_clients++;
    return true;
}

client_info *search_client(client_database *c_db, int fd, int *pos)
{
    for (int i = 0; i < c_db->nr_clients; i++)
    {
        if (c_db->clients_information[i].fd == fd)
        {
            if (pos)
                *pos = i;
            return &c_db->clients_information[i];
        }
    }
    return NULL;
}

bool disconnect_client(client_database *c_db, int fd)
{
    client_info *client_addr = search_client(c_db, fd, NULL);
    if (!client_addr)
        return false;

    printf("Client %s disconnected.\n", client_addr->client_id);
    client_addr->active = 0;
    client_addr->fd = 0;
    return true;
}
