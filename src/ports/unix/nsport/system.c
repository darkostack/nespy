#include "port_unix.h"
#include <errno.h>

void unix_process_update(void)
{
    fd_set read_fds;
    fd_set write_fds;
    fd_set error_fds;
    int max_fd = -1;
    int rval;
    struct timeval timeout;

    timeout.tv_sec = 0;
    timeout.tv_usec = 0;

    FD_ZERO(&read_fds);
    FD_ZERO(&write_fds);
    FD_ZERO(&error_fds);

    unix_uart_update_fd_set(&read_fds, &write_fds, &error_fds, &max_fd);
    unix_radio_update_fd_set(&read_fds, &write_fds, &max_fd);

    rval = select(max_fd + 1, &read_fds, &write_fds, &error_fds, &timeout);

    if ((rval < 0) && (errno != EINTR))
    {
        perror("select");
        exit(EXIT_FAILURE);
    }

    unix_uart_process();
    unix_radio_process();
    rtimer_alarm_process();
    etimer_pending_process();
}
