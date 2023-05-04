#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/socket.h>
#include "news_protocol.h"
#include <stdint.h>

#define MAX_BUFFER

int recv_tcp_packet(int fd, char* buffer_addr, int expcted_size)
{
    uint32_t bytes_to_read = expcted_size;
    uint32_t bytes_already_read = 0;

    while (bytes_to_read > 0)
    {
        int rc = recv(fd, buffer_addr + bytes_already_read, bytes_to_read, 0);
        if (rc < 0) {
            return rc;
        } else if (rc == 0) {
            break;
        }
        bytes_to_read-= rc;
        bytes_already_read += rc;

    }
    return bytes_already_read;
}

int send_tcp_packet(int fd, char* buffer_addr, int size_to_sent)
{
    uint32_t bytes_to_send = size_to_sent;
    uint32_t bytes_already_sent = 0;

    while (bytes_to_send > 0)
    {
        int rc = send(fd, buffer_addr + bytes_already_sent, bytes_to_send, 0);
        if (rc < 0) {
            return rc;
        } else if (rc == 0) {
            break;
        }

        bytes_to_send-=rc;
        bytes_already_sent +=rc;
    }

    return bytes_already_sent;
}