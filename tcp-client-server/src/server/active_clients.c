#include "server.h"
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

bool add_client(client_info **list_addr, char id[10], int nr_clients)
{
    client_info *list = *list_addr;
    if (!list)
    {
        list = malloc(sizeof(client_info));
    }
    else
    {
        for (int i = 0; i < nr_clients; i++)
        {
            if (strncmp(list[i].client_id, id, strlen(id)) == 0 &&
                strlen(id) == strlen(list[i].client_id))
            {
                return false;
            }
        }
        client_info* new = realloc(list, sizeof(client_info) * (nr_clients + 1));
        if(new == NULL)
            return false;
        list = new;
    }

    strncpy(list[nr_clients].client_id, id, 10);
    *list_addr = list;
    return true;
}

bool remove_client(client_info **list_addr, int pos, int nr_cli)
{
    client_info *list = *list_addr;
    nr_cli--;
    for (int i = pos; i < nr_cli; i++)
    {
        memcpy(&list[i], &list[i + 1], sizeof(client_info));
    }
    list = realloc(list, sizeof(client_info) * nr_cli);
    *list_addr = list;
    return true;
}
