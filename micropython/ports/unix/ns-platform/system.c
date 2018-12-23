#include <assert.h>
#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "ns/tasklet.h"
#include "ns/platform/alarm-milli.h"
#include "ns/platform/alarm-micro.h"
#include "ns/platform/radio.h"
#include "ns/platform/system.h"
#include "platform-unix.h"

uint32_t g_node_id = 1;

static volatile bool s_terminate = false;

static void
handle_signal(int signal)
{
    (void)signal;
    s_terminate = true;
}

void
ns_plat_sys_init(uint32_t id)
{
    signal(SIGTERM, &handle_signal);
    signal(SIGHUP, &handle_signal);

    g_node_id = id;

    if (g_node_id >= WELLKNOWN_NODE_ID) {
        fprintf(stderr, "Invalid node id: %u\n", id);
        exit(EXIT_FAILURE);
    }

    plat_alarm_init();
    plat_radio_init();
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
    plat_radio_update_fd_set(&read_fds, &write_fds, &max_fd);
    plat_alarm_update_timeout(&timeout);

    if (!ns_tasklet_are_pending(instance)) {
        rval = select(max_fd + 1, &read_fds, &write_fds, &error_fds, &timeout);
        if ((rval < 0) && (errno != EINTR)) {
            perror("select");
            exit(EXIT_FAILURE);
        }
    }

    if (s_terminate) {
        exit(0);
    }

    plat_uart_process();
    plat_radio_process(instance);
    plat_alarm_process(instance);
}
