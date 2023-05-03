#include "server.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

struct topic *search_topic(client_database *c_db, char name[50])
{
    for (int i = 0; i < c_db->nr_topics; i++)
    {
        if (strncmp(c_db->exsitent_topics[i].topic, name, 50) == 0 &&
            strlen(name) == strlen(c_db->exsitent_topics[i].topic))
        {
            return &c_db->exsitent_topics[i];
        }
    }
    return NULL;
}

void create_topic(client_database *c_db, news_packet *info_addr)
{
    size_t topic_size = sizeof(struct topic);
    if (!c_db->exsitent_topics)
    {
        c_db->exsitent_topics = malloc(topic_size);
    }
    else
        c_db->exsitent_topics = realloc(c_db->exsitent_topics, topic_size * (c_db->nr_topics + 1));

    memcpy(c_db->exsitent_topics[c_db->nr_topics].topic, info_addr->topic, 50);
    c_db->exsitent_topics[c_db->nr_topics].nr_subscribers = 0;
    c_db->exsitent_topics[c_db->nr_topics].subscribers = NULL;
    c_db->nr_topics++;
}

bool add_client_to_topic(struct topic *topic_addr, int pos, news_packet *info_addr)
{
    size_t size_cli_add = sizeof(struct topics_clients);
    if (!topic_addr->subscribers)
    {
        topic_addr->subscribers = malloc(size_cli_add);
    }
    else
    {
        topic_addr->subscribers = realloc(topic_addr->subscribers, size_cli_add * (topic_addr->nr_subscribers + 1));
    }

    struct topics_clients *sub_addr = &topic_addr->subscribers[topic_addr->nr_subscribers];
    topic_addr->nr_subscribers++;
    sub_addr->pos_in_client_vector = pos;
    sub_addr->sf = info_addr->un.req.sf;
    sub_addr->next_mess = NULL;
    return true;
}

void remove_client_from_topic(struct topic *topic_addr, int pos)
{
    int i;
    for (i = 0; i < topic_addr->nr_subscribers; i++)
    {
        if (topic_addr->subscribers[i].pos_in_client_vector == pos)
            break;
    }

    topic_addr->nr_subscribers--;
    for (int j = i; j < topic_addr->nr_subscribers; j++)
    {
        topic_addr->subscribers[j] = topic_addr->subscribers[j + 1];
    }
    size_t size_cli_add = sizeof(struct topics_clients);
    topic_addr->subscribers = realloc(topic_addr->subscribers, size_cli_add * (topic_addr->nr_subscribers));
    return;
}
