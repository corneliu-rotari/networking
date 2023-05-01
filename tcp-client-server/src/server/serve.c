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
    // rc = bind(udp_socket, (const struct sockaddr *)&server_addr, sizeof(server_addr));
    // DIE(rc < 0, "UDP bind");

    struct pollfd *poll_fds = init_poll(&nr_fds);
    poll_fds = add_to_poll(poll_fds, tcp_socket, &nr_fds);
    poll_fds = add_to_poll(poll_fds, udp_socket, &nr_fds);

    rc = listen(tcp_socket, 5);
    DIE(rc < 0, "Listen");

    bool exit_value = false;


    while (true)
    {
        if (exit_value)
            break;
        
        rc = poll(poll_fds, nr_fds, -1);
        DIE(rc < 0, "Poll");

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
                        exit_value = true;
                    }
                }
                else if (poll_fds[i].fd == tcp_socket)
                {
                    struct sockaddr_in cli_addr;
                    char buf[MAX_LEN_BUFF];
                    int rec_len;
                    CSP_packet packet;
                    socklen_t cli_len = sizeof(cli_addr);
                    int newsockfd = accept(tcp_socket, (struct sockaddr *)&cli_addr, &cli_len);
                    poll_fds = add_to_poll(poll_fds, newsockfd, &nr_fds);
                    rec_len = recv(newsockfd,(void*)&buf, MAX_LEN_BUFF, 0);
                    
                    printf("New client %s connected from %s:%hu.\n"
                        ,buf, inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));
                }
                else if (poll_fds[i].fd == udp_socket)
                {
                    // printf("Something from UDP\n");
                }
                else
                {
                    // printf("Something from TCP at %d\n", poll_fds[i].fd);
                    poll_fds = remove_poll(poll_fds, poll_fds[i].fd, &nr_fds, i);
                }
                break;
            }
        }
    }

    destory_poll(poll_fds, nr_fds);
    exit(EXIT_SUCCESS);
}
