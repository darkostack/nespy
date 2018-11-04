#ifndef NSPORT_PORT_UNIX_H_
#define NSPORT_PORT_UNIX_H_

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

void rtimer_alarm_process(void);
void etimer_pending_process(void);

void unix_radio_update_fd_set(fd_set *read_fd_set, fd_set *write_fd_set, int *max_fd);
void unix_radio_process(void);

void unix_uart_restore(void);
void unix_uart_enable(void);
void unix_uart_disable(void);
void unix_uart_update_fd_set(fd_set *read_fd_set, fd_set *write_fd_set, fd_set *error_fd_set, int *max_fd);
void unix_uart_process(void);

void unix_process_update(void);

#endif // NSPORT_PORT_UNIX_H_
