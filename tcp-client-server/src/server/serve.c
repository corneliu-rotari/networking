#include "server.h"

int main(int argc, char const *argv[])
{
    DIE(argc != 2, "[Usage] : ./server <PORT_NUMBER>\n");
    setvbuf(stdout, NULL, _IONBF, BUFSIZ);

    int rc, nr_fds;
    int tcp_socket, udp_socket;
    struct sockaddr_in server_addr;

    uint16_t server_port;
    rc = sscanf(argv[1], "%hu", &server_port);
    DIE(rc != 1, "Port invalid");

    tcp_socket = socket(AF_INET, SOCK_STREAM, 0);
    DIE(tcp_socket < 0, "TCP listen socket");
    int enable = 1;
    rc = setsockopt(tcp_socket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));
    DIE(rc < 0, "Option Reuse");
    rc = setsockopt(tcp_socket, IPPROTO_TCP, TCP_NODELAY, &enable, sizeof(int));
    DIE(rc < 0, "Option Nodelay");

    udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
    DIE(udp_socket < 0, "UDP Socket");

    memset(&server_addr, 0, sizeof(struct sockaddr_in));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    rc = bind(tcp_socket, (const struct sockaddr *)&server_addr, sizeof(server_addr));
    DIE(rc < 0, "TCP bind");
    rc = bind(udp_socket, (const struct sockaddr *)&server_addr, sizeof(server_addr));
    DIE(rc < 0, "UDP bind");

    struct pollfd *poll_fds = init_poll(&nr_fds);
    poll_fds = add_to_poll(poll_fds, tcp_socket, &nr_fds);
    poll_fds = add_to_poll(poll_fds, udp_socket, &nr_fds);

    rc = listen(tcp_socket, 5);
    DIE(rc < 0, "Listen");

    // -----------------------------------------------------------------------------------------------

    client_database *c_db = malloc(sizeof(client_database));
    c_db->clients_information = NULL;
    c_db->nr_clients = 0;
    c_db->exsitent_topics = NULL;
    c_db->nr_topics = 0;

    bool exit_value = false;

    while (true)
    {
        if (exit_value)
            break;

        int nr_events = poll(poll_fds, nr_fds, -1);
        DIE(nr_events < 0, "Poll");

        for (int i = 0; i < nr_fds; i++)
        {
            if (poll_fds[i].revents & POLLIN)
            {
                if (poll_fds[i].fd == STDIN_FILENO)
                {
                    char buff[MAX_LEN_BUFF];
                    if (!fgets(buff, MAX_LEN_BUFF, stdin))
                        DIE(true, "An fgets error");

                    if (isExit(buff))
                    {
                        exit_value = true;
                        break;
                    }
                }
                else if (poll_fds[i].fd == tcp_socket)
                {
                    connect_tcp_client_to_server(tcp_socket, c_db, &nr_fds, &poll_fds);
                }
                else if (poll_fds[i].fd == udp_socket)
                {
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
                    memcpy(&send_packet.un.rep.content, &recv_packet, sizeof(recv_packet));

                    struct topic *topic_addr = search_topic(c_db, recv_packet.topic);

                    if (!topic_addr)
                        continue;

                    for (int j = 0; j < topic_addr->nr_subscribers; j++)
                    {
                        int pos_in_cli_vec = topic_addr->subscribers[j].pos_in_client_vector;
                        if (c_db->clients_information[pos_in_cli_vec].active)
                        {
                            send_tcp_packet(c_db->clients_information[pos_in_cli_vec].fd, (char *)&send_packet,
                                            NEWS_PACKET_HEADER_SIZE + size_packet);
                        }
                        else if (topic_addr->subscribers[j].sf)
                        {
                            store_packet(&topic_addr->subscribers[j], &send_packet);
                        }
                    }
                }
                else
                {
                    news_packet recv_packet;
                    rc = recv_tcp_packet(poll_fds[i].fd, (char *)&recv_packet, NEWS_PACKET_HEADER_SIZE);
                    DIE(rc < 0, "Receive from client");

                    if (rc == 0)
                    {
                        disconnect_client(c_db, poll_fds[i].fd);
                        poll_fds = remove_poll(poll_fds, poll_fds[i].fd, &nr_fds, i);
                    }
                    else
                    {
                        rc = recv_tcp_packet(poll_fds[i].fd, (char *)&recv_packet.un, recv_packet.size);
                        if (recv_packet.packet_type == NEWS_PACK_REQ)
                        {
                            parse_and_exec_client_request(c_db, &recv_packet, poll_fds[i].fd);
                        }
                    }
                }
            }
        }
    }

    for (int i = 0; i < c_db->nr_topics; i++)
    {
        for (int j = 0; j < c_db->exsitent_topics[i].nr_subscribers; j++)
        {
            destory_list(&c_db->exsitent_topics[i].subscribers[j]);
        }
        free(c_db->exsitent_topics[i].subscribers);
    }
    free(c_db->exsitent_topics);
    free(c_db->clients_information);
    free(c_db);
    destory_poll(poll_fds, nr_fds);
    exit(EXIT_SUCCESS);
}
