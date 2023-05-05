#include "server.h"

void run_server(int udp_socket, int tcp_socket, client_database *c_db)
{
    int nr_fds;

    struct pollfd *poll_fds = init_poll(&nr_fds);
    poll_fds = add_to_poll(poll_fds, tcp_socket, &nr_fds);
    poll_fds = add_to_poll(poll_fds, udp_socket, &nr_fds);

    while (true)
    {
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
                        destory_poll(poll_fds, nr_fds);
                        return;
                    }
                }
                else if (poll_fds[i].fd == tcp_socket)
                {
                    connect_tcp_client_to_server(tcp_socket, c_db, &nr_fds, &poll_fds);
                }
                else if (poll_fds[i].fd == udp_socket)
                {
                    recv_udp_news(c_db, udp_socket);
                }
                else
                {
                    news_packet recv_packet;
                    int rc = recv_tcp_packet(poll_fds[i].fd, (char *)&recv_packet, NEWS_PACKET_HEADER_SIZE);
                    DIE(rc < 0, "Receive from client");

                    if (rc == 0)
                    {
                        DIE(!disconnect_client(c_db, poll_fds[i].fd), "There is no client");
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
}

int main(int argc, char const *argv[])
{
    DIE(argc != 2, "[Usage] : ./server <PORT_NUMBER>\n");

    setvbuf(stdout, NULL, _IONBF, BUFSIZ);

    int rc, tcp_socket, udp_socket;
    struct sockaddr_in server_addr;

    uint16_t server_port;
    rc = sscanf(argv[1], "%hu", &server_port);
    DIE(rc != 1, "Port invalid");

    // Create sockets
    udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
    DIE(udp_socket < 0, "UDP Socket");

    tcp_socket = socket(AF_INET, SOCK_STREAM, 0);
    DIE(tcp_socket < 0, "TCP main socket");

    // Set options for socket use
    int enable = 1;
    rc = setsockopt(tcp_socket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));
    DIE(rc < 0, "Option Reuse tcp");
    rc = setsockopt(tcp_socket, IPPROTO_TCP, TCP_NODELAY, &enable, sizeof(int));
    DIE(rc < 0, "Option Nodelay tcp");

    // Create server address
    memset(&server_addr, 0, sizeof(struct sockaddr_in));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // Bind the sockets to the address
    rc = bind(tcp_socket, (const struct sockaddr *)&server_addr, sizeof(server_addr));
    DIE(rc < 0, "TCP bind");
    rc = bind(udp_socket, (const struct sockaddr *)&server_addr, sizeof(server_addr));
    DIE(rc < 0, "UDP bind");

    rc = listen(tcp_socket, 5);
    DIE(rc < 0, "Listen");

    // This stores all the information about clinets
    client_database *c_db = malloc(sizeof(client_database));
    c_db->clients_information = NULL;
    c_db->nr_clients = 0;
    c_db->exsitent_topics = NULL;
    c_db->nr_topics = 0;

    // Main entry point
    run_server(udp_socket, tcp_socket, c_db);

    // Clean up the memory
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
    exit(EXIT_SUCCESS);
}
