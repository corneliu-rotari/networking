#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include <errno.h>
#include <ctype.h>
#include <poll.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <sys/poll.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "../lib/lib_tcp_utils..h"

int main(int argc, char const *argv[])
{
    DIE(argc != 4, "[Usage] : ./subscriber <ID_CLIENT> <IP_SERVER> <PORT_SERVER>\n");
    setvbuf(stdout, NULL, _IONBF, BUFSIZ);

    int rc;
    CSP_packet packet;

    // Parse ID, IP, Port
    const char *client_id = argv[1];
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

    // TODO: TCP multiplex
    int nr_fds = 0;

    rc = connect(serverfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    DIE(rc < 0, "Connect");
    send(serverfd, (void *)client_id, sizeof(client_id), 0);

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

            if (isExit(buff))
                break;

            if (isSubscribe(buff))
            {
                
            }
        }
        else
        {
            printf("Recived something\n");
            break;
        }
    }

    destory_poll(poll_fds, nr_fds);
    exit(EXIT_SUCCESS);
}
