#include "server.h"

void connect_tcp_client_to_server(int tcp_socket, client_database *c_db, int *nr_fds_addr, struct pollfd **poll_fds_addr)
{
    int rc = 0;
    struct pollfd *poll_fds = *poll_fds_addr;
    struct sockaddr_in cli_addr;
    news_packet recv_packet;
    memset(&recv_packet, 0, sizeof(news_packet));

    socklen_t cli_len = sizeof(cli_addr);
    int newsockfd = accept(tcp_socket, (struct sockaddr *)&cli_addr, &cli_len);

    rc = recv_tcp_packet(newsockfd, (char *)&recv_packet, sizeof(uint8_t) + sizeof(uint16_t));
    DIE(rc < -1, "Recive tcp listen fd");

    rc = recv_tcp_packet(newsockfd, (char *)&recv_packet.un, recv_packet.size);
    DIE(rc < -1, "Recive tcp listen fd");

    if (recv_packet.packet_type != NEWS_PACK_ID)
    {
        close(newsockfd);
    }

    if (add_client_to_client_list(c_db, recv_packet.un.id, newsockfd))
    {
        poll_fds = add_to_poll(poll_fds, newsockfd, nr_fds_addr);
        printf("New client %s connected from %s:%hu.\n",
               recv_packet.un.id, inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));
    }
    else
    {
        close(newsockfd);
        printf("Client %s already connected.\n", recv_packet.un.id);
    }

    *poll_fds_addr = poll_fds;
}

void parse_and_exec_client_request(client_database *c_db, news_packet *recv_packet_addr, int clinet_fd)
{
    news_packet send_packet;
    send_packet.packet_type = NEWS_PACK_ACK;
    send_packet.size = 0;

    if (recv_packet_addr->un.req.type_action == NEWS_TYPE_SUB)
    {
        struct topic *topic_addr = search_topic(c_db, recv_packet_addr->un.req.topic);
        if (!topic_addr)
        {
            create_topic(c_db, recv_packet_addr);
            topic_addr = &c_db->exsitent_topics[c_db->nr_topics - 1];
        }

        int pos;
        search_client(c_db, clinet_fd, &pos);

        add_client_to_topic(topic_addr, pos, recv_packet_addr);
    }
    else if (recv_packet_addr->un.req.type_action == NEWS_TYPE_UNSUB)
    {
        struct topic *topic_addr = search_topic(c_db, recv_packet_addr->un.req.topic);
        if (topic_addr)
        {
            int pos;
            search_client(c_db, clinet_fd, &pos);
            remove_client_from_topic(topic_addr, pos);
        }
    }

    send_tcp_packet(clinet_fd, (char *)&send_packet, NEWS_PACKET_HEADER_SIZE);
}

void recv_udp_news(client_database *c_db, int udp_socket)
{
    int rc;
    source_packet recv_packet;
    memset(&recv_packet, 0, sizeof(source_packet));
    struct sockaddr_in udp_client_addr;
    socklen_t addr_len = sizeof(udp_client_addr);

    rc = recvfrom(udp_socket, (void *)&recv_packet, sizeof(source_packet), 0, (struct sockaddr *)&udp_client_addr, &addr_len);
    DIE(rc < 0, "Receive UDP packet");

    news_packet send_packet;
    memset(&send_packet, 0, sizeof(news_packet));
    send_packet.packet_type = NEWS_PACK_REP;
    uint16_t size_packet = sizeof(recv_packet) + sizeof(udp_client_addr.sin_port) + sizeof(udp_client_addr.sin_addr);
    send_packet.size = htons(size_packet);
    send_packet.un.rep.ip_udp = udp_client_addr.sin_addr;
    send_packet.un.rep.port_udp = udp_client_addr.sin_port;
    memcpy(&send_packet.un.rep.content, &recv_packet, rc);

    struct topic *topic_addr = search_topic(c_db, recv_packet.topic);

    if (!topic_addr)
        return;

    for (int j = 0; j < topic_addr->nr_subscribers; j++)
    {
        int pos_in_cli_vec = topic_addr->subscribers[j].pos_in_client_vector;
        if (c_db->clients_information[pos_in_cli_vec].active)
        {
            rc = send_tcp_packet(c_db->clients_information[pos_in_cli_vec].fd, (char *)&send_packet,
                                 NEWS_PACKET_HEADER_SIZE + size_packet);
            DIE(rc < 0, "Send packet to subscriber");
        }
        else if (topic_addr->subscribers[j].sf)
        {
            store_packet(&topic_addr->subscribers[j], &send_packet);
        }
    }
}
