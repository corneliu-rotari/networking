#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include <errno.h>
#include <ctype.h>
#include <poll.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <unistd.h>

#include <sys/poll.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "../lib/lib_tcp_utils..h"
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

    client_database *clients_data = malloc(sizeof(client_database));
    clients_data->clients_information = NULL;
    clients_data->nr_clients = 0;

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
                    fgets(buff, MAX_LEN_BUFF, stdin);

                    if (isExit(buff))
                    {
                        destory_poll(poll_fds, nr_fds);
                        exit(EXIT_SUCCESS);
                    }

                    break;
                }
                else if (poll_fds[i].fd == tcp_socket)
                {
                    struct sockaddr_in cli_addr;
                    news_packet recv_packet;

                    socklen_t cli_len = sizeof(cli_addr);
                    int newsockfd = accept(tcp_socket, (struct sockaddr *)&cli_addr, &cli_len);

                    rc = recv(newsockfd, (void *)&recv_packet, sizeof(recv_packet), 0);
                    DIE(rc < -1, "Recive tcp listen fd");

                    if (connect_client(clients_data, recv_packet.un.req.id, newsockfd))
                    {
                        poll_fds = add_to_poll(poll_fds, newsockfd, &nr_fds);
                        printf("New client %s connected from %s:%hu.\n",
                               recv_packet.un.req.id, inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));
                    }
                    else
                    {
                        close(newsockfd);
                        printf("Client %s already connected.\n", recv_packet.un.req.id);
                    }
                    break;
                }
                else if (poll_fds[i].fd == udp_socket)
                {
                    source_packet recv_packet;
                    struct sockaddr_in udp_client_addr;
                    socklen_t addr_len = sizeof(udp_client_addr);

                    recvfrom(udp_socket, (void *)&recv_packet, sizeof(source_packet), 0, (struct sockaddr *)&udp_client_addr, &addr_len);

                    news_packet send_packet;
                    memset(&send_packet, 0, sizeof(news_packet));
                    send_packet.type = NEWS_REP;
                    memcpy(&send_packet.topic, &recv_packet.topic, sizeof(send_packet.topic));
                    send_packet.un.rep.type = recv_packet.content.type;
                    send_packet.un.rep.ip_udp = udp_client_addr.sin_addr;
                    send_packet.un.rep.port_udp = udp_client_addr.sin_port;
                    memcpy(&send_packet.un.rep.messege, &recv_packet.content.payload, sizeof(recv_packet.content.payload));

                    for (int j = 0; j < clients_data->nr_clients; j++)
                    {
                        for (int k = 0; k < clients_data->clients_information[j].nr_subscribed; k++)
                        {
                            // printf("%s, %s\n", clients_data->clients_information[j].subscribed_to[k].topic, send_packet.topic);

                            if (strncmp(clients_data->clients_information[j].subscribed_to[k].topic, send_packet.topic, strlen(send_packet.topic)) == 0) {
                                if (clients_data->clients_information[j].active)
                                {
                                    send(clients_data->clients_information[j].fd, &send_packet, sizeof(send_packet), 0);
                                }
                                else if (clients_data->clients_information[j].subscribed_to[k].sf) {
                                    // TODO Add to store and forward;
                                }
                                break;
                            }
                        }

                    }

                    break;
                }
                else
                {
                    news_packet recv_packet;
                    rc = recv(poll_fds[i].fd, &recv_packet, sizeof(recv_packet), 0);
                    DIE(rc < 0, "Receive from client");

                    if (rc == 0)
                    {
                        disconnect_client(clients_data, poll_fds[i].fd);
                        poll_fds = remove_poll(poll_fds, poll_fds[i].fd, &nr_fds, i);
                    }
                    else
                    {
                        if (recv_packet.type == NEWS_REQ)
                        {
                            if (recv_packet.un.req.type_action == NEWS_SUB)
                            {
                                add_topic_to_client(clients_data, poll_fds[i].fd, &recv_packet);
                                // for (int j = 0; j < clients_data->nr_clients; j++)
                                // {
                                //     printf("Client : %s [active] : %d\n",
                                //            clients_data->clients_information[j].client_id, clients_data->clients_information[j].active);
                                //     for (int k = 0; k < clients_data->clients_information[j].nr_subscribed; k++)
                                //     {
                                //         printf("    Topic %s with sf %d\n",
                                //                clients_data->clients_information[j].subscribed_to[k].topic, clients_data->clients_information[j].subscribed_to[k].sf);
                                //     }
                                // }
                            }
                            else if (recv_packet.un.req.type_action == NEWS_UNSUB)
                            {
                                // TODO Add unsubscribe 
                                printf("Unsubscribed\n");
                            }
                        }
                    }
                    break;
                }
            }
        }
    }
}
