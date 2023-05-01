#include <poll.h>
#include <sys/poll.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>

struct pollfd *init_poll(int *nr_fds)
{
    struct pollfd *poll_fds = malloc(sizeof(struct pollfd));
    poll_fds[0].fd = STDIN_FILENO;
    poll_fds[0].events = POLLIN;
    *nr_fds = 1;
    return poll_fds;
}

struct pollfd *add_to_poll(struct pollfd *poll_fds, int fd, int *nr_fds)
{
    int nr = *nr_fds;
    nr++;
    struct pollfd *aux = realloc(poll_fds, sizeof(struct pollfd) * nr);
    aux[nr - 1].fd = fd;
    aux[nr - 1].events = POLLIN;
    *nr_fds = nr;
    return aux;
}

struct pollfd *remove_poll(struct pollfd *poll_fds, int fd, int *nr_fds, int pos)
{
    close(fd);
    int nr = (*nr_fds) - 1;
    for (int i = pos; i < nr; i++)
    {
        poll_fds[i] = poll_fds[i + 1];
    }
    *nr_fds = nr;

    return realloc(poll_fds, sizeof(struct pollfd) * nr);
}

void destory_poll(struct pollfd *poll_fds, int nr_fds)
{
    for (int i = 1; i < nr_fds; i++)
    {
        shutdown(poll_fds[i].fd, SHUT_RDWR);
        close(poll_fds[i].fd);
    }
    free(poll_fds);
}
