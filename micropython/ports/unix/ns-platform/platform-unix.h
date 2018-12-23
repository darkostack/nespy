#ifndef NS_PLATFORM_UNIX_H_
#define NS_PLATFORM_UNIX_H_

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <poll.h>
#include <signal.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>
#define POLL poll

#include "core/core-config.h"
#include "platform-config.h"

#include "ns/instance.h"

enum
{
    WELLKNOWN_NODE_ID = 34,
};

extern uint32_t g_node_id;

void
plat_alarm_init(void);

void
plat_alarm_update_timeout(struct timeval *tv);

void 
plat_alarm_process(ns_instance_t instance);

uint64_t
plat_alarm_get_now(void);

void
plat_random_init(void);

void
plat_uart_restore(void);

void
plat_uart_update_fd_set(fd_set *read_fd, fd_set *write_fd, fd_set *error_fd, int *max_fd);

void
plat_uart_process(void);

void
plat_radio_init(void);

void
plat_radio_deinit(void);

void
plat_radio_update_fd_set(fd_set *read_fd, fd_set *write_fd, int *max_fd);

void
plat_radio_process(ns_instance_t instance);

#endif // NS_PLATFORM_UNIX_H_
