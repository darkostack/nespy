#include "port_unix.h"
#include <errno.h>

extern void rtimer_alarm_process(void);
extern void etimer_pending_process(void);
extern void unix_radio_update_fd_set(fd_set *read_fd_set, fd_set *write_fd_set, int *max_fd);
extern void unix_radio_process(void);

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

    unix_radio_update_fd_set(&read_fds, &write_fds, &max_fd);

    rval = select(max_fd + 1, &read_fds, &write_fds, &error_fds, &timeout);

    if ((rval < 0) && (errno != EINTR))
    {
        perror("select");
        exit(EXIT_FAILURE);
    }

    unix_radio_process();
    rtimer_alarm_process();
    etimer_pending_process();
}
