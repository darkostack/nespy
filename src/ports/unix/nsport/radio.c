#include "port_unix.h"

#define DEFAULT_PORT 9000
static int s_sock_fd;

void unix_radio_init(void)
{
    struct sockaddr_in sockaddr;
    memset(&sockaddr, 0, sizeof(sockaddr));

    sockaddr.sin_family = AF_INET;
    sockaddr.sin_port = htons(DEFAULT_PORT);
    sockaddr.sin_addr.s_addr = INADDR_ANY;

    s_sock_fd = (int)socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    if (s_sock_fd == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    if (bind(s_sock_fd, (struct sockaddr *)&sockaddr, sizeof(sockaddr)) == -1) {
        perror("bind");
        exit(EXIT_FAILURE);
    }
}

void unix_radio_update_fd_set(fd_set *read_fd_set, fd_set *write_fd_set, int *max_fd)
{
    if (read_fd_set != NULL) {
        FD_SET(s_sock_fd, read_fd_set);
        if (max_fd != NULL && *max_fd < s_sock_fd) {
            *max_fd = s_sock_fd;
        }
    }
    if (write_fd_set != NULL) {
        FD_SET(s_sock_fd, write_fd_set);
        if (max_fd != NULL && *max_fd < s_sock_fd) {
            *max_fd = s_sock_fd;
        }
    }
}

void unix_radio_process(void)
{
    const int     flags  = POLLIN | POLLRDNORM | POLLERR | POLLNVAL | POLLHUP;
    struct pollfd pollfd = {s_sock_fd, flags, 0};

    if (POLL(&pollfd, 1, 0) > 0 && (pollfd.revents & flags) != 0)
    {
        // received
    }
}
