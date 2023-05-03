#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include <errno.h>
#include <ctype.h>
#include <poll.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <math.h>

#include <sys/poll.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "../lib/lib_tcp_utils.h"

int main(int argc, char const *argv[])
{
    DIE(argc != 4, "[Usage] : ./subscriber <ID_CLIENT> <IP_SERVER> <PORT_SERVER>\n");
    setvbuf(stdout, NULL, _IONBF, BUFSIZ);

    int rc;
    news_packet send_packet;
    memset(&send_packet, 0, sizeof(news_packet));

    // Create request packet
    send_packet.type = NEWS_REQ;
    memcpy(send_packet.un.req.id, argv[1], strlen(argv[1]) + 1);

    uint16_t server_port;
    rc = sscanf(argv[3], "%hu", &server_port);
    DIE(rc != 1, "Port invalid");

    // Create socket to communicate with the server
    int serverfd = socket(AF_INET, SOCK_STREAM, 0);
    DIE(serverfd < 0, "Socket");

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(struct sockaddr_in));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    inet_pton(AF_INET, argv[2], &server_addr.sin_addr.s_addr);

    int nr_fds = 0;

    rc = connect(serverfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    DIE(rc < 0, "Connect");

    rc = send(serverfd, (void *)&send_packet, sizeof(send_packet), 0);
    DIE(rc < 0, "Send packet");

    struct pollfd *poll_fds = init_poll(&nr_fds);
    poll_fds = add_to_poll(poll_fds, serverfd, &nr_fds);

    while (true)
    {
        rc = poll(poll_fds, nr_fds, -1);
        DIE(rc < 0, "Poll");

        if (poll_fds[0].revents & POLLIN)
        {
            char buff[MAX_LEN_BUFF];
            fgets(buff, MAX_LEN_BUFF, stdin);
            char command[20];
            char topic[51] = {'\0'};

            sscanf(buff, "%s", command);

            if (isExit(command))
                break;

            if (isSubscribe(command))
            {
                sscanf(buff, "%s %s %hhu", command, topic, &send_packet.un.req.sf);
                memcpy(send_packet.topic, topic, 50);
                send_packet.un.req.type_action = NEWS_SUB;
                send(serverfd, (void *)&send_packet, sizeof(send_packet), 0);
                recv(serverfd, &send_packet, sizeof(send_packet),0);
                printf("Subscribed to topic.\n");
            }
            else if (isUnsubscribe(buff))
            {
                sscanf(buff, "%s %s", command, topic);
                memcpy(send_packet.topic, topic, 50);
                send_packet.un.req.type_action = NEWS_UNSUB;
                send(serverfd, (void *)&send_packet, sizeof(send_packet), 0);
                
                printf("Unsubscribed from topic.\n");
            }
            else
            {
                printf("Invalid command. Try again\n");
                continue;
            }
        }
        else
        {
            news_packet recv_packet;
            rc = recv(serverfd, (void *)&recv_packet, sizeof(recv_packet), 0);
            DIE(rc < 0, "Recived from server");

            if (rc == 0)
                break;
            else
            {
                if (recv_packet.type == NEWS_REP)
                {
                    printf("%s:%hu - %s - %s - ",
                           inet_ntoa(recv_packet.un.rep.ip_udp), ntohs(recv_packet.un.rep.port_udp),
                           recv_packet.topic, convert_type(recv_packet.un.rep.type));

                    switch (recv_packet.un.rep.type)
                    {
                    case 0:
                    {
                        uint8_t sign = *(uint8_t *)recv_packet.un.rep.messege;
                        if (sign == 1)
                            printf("-");
                        printf("%u\n", ntohl(*((uint32_t *)(recv_packet.un.rep.messege + 1))));
                        break;
                    }
                    case 1:
                    {
                        uint16_t short_real = ntohs(*(uint16_t *)recv_packet.un.rep.messege);
                        printf("%hu.", short_real / 100);
                        uint16_t right = short_real % 100;
                        if (right < 10)
                            printf("0");
                        printf("%hu\n",right);
                        break;
                    }
                    case 2:
                    {
                        uint8_t sign = *(uint8_t *)recv_packet.un.rep.messege;
                        uint32_t number = ntohl(*(uint32_t*) (recv_packet.un.rep.messege + 1));
                        uint8_t power = *(recv_packet.un.rep.messege+ sizeof(uint32_t)  + 1);
                        int mult_ten = (int) pow(10, power);
                        int left = number / mult_ten;
                        int right = number % mult_ten;
                        
                        if (sign == 1)
                            printf("-");
                        printf("%d.", left);

                        if (right < 10)
                            printf("00");
                        else if (right < 100)
                            printf("0");
                        
                        printf("%d\n",right);

                        break;
                    }
                    case 3: {
                        printf("%s\n", recv_packet.un.rep.messege);
                        break;
                    }
                    default:
                        break;
                    }
                }
            }
        }
    }

    destory_poll(poll_fds, nr_fds);
    exit(EXIT_SUCCESS);
}
