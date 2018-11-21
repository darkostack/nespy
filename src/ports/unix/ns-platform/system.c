#include <assert.h>
#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "ns-platform/platform-unix.h"
#include "ns/include/platform/alarm.h"
#include "ns/include/platform/system.h"

uint32_t node_id = 1;

static volatile bool terminate = false;

static void
handle_signal(int signal)
{
    (void)signal;
    terminate = true;
}

void
ns_plat_sys_init(uint32_t id)
{
    signal(SIGTERM, &handle_signal);
    signal(SIGHUP, &handle_signal);

    node_id = id;

    if (node_id >= WELLKNOWN_NODE_ID) {
        fprintf(stderr, "Invalid node id: %u\n", id);
        exit(EXIT_FAILURE);
    }

    plat_alarm_init();
    // TODO: plat_radio_init();
    plat_random_init();
}

void
ns_plat_sys_process_drivers(ns_instance_t instance)
{
    fd_set read_fds;
    fd_set write_fds;
    fd_set error_fds;
    int max_fd = -1;
    struct timeval timeout;
    int rval;

    FD_ZERO(&read_fds);
    FD_ZERO(&write_fds);
    FD_ZERO(&error_fds);

    plat_uart_update_fd_set(&read_fds, &write_fds, &error_fds, &max_fd);
    // TODO: plat_radio_update_fd_set(&read_fds, &write_fds, &max_fd);
    plat_alarm_update_timeout(&timeout);

    rval = select(max_fd + 1, &read_fds, &write_fds, &error_fds, &timeout);
    if ((rval < 0) && (errno != EINTR)) {
        perror("select");
        exit(EXIT_FAILURE);
    }

    if (terminate) {
        exit(0);
    }

    plat_uart_process();
    // TODO: plat_radio_process(instance);
    plat_alarm_process(instance);
}
