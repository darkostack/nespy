#include "ns/contiki.h"
#include "ns/modules/nstd.h"
#include "ns/modules/platform.h"
#include "port_unix.h"
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

static uint8_t s_receive_buffer[128];
static const uint8_t *s_write_buffer;
static uint16_t s_write_length;
static int s_in_fd;
static int s_out_fd;

static struct termios original_stdin_termios;
static struct termios original_stdout_termios;

static void restore_stdin_termios(void)
{
    tcsetattr(s_in_fd, TCSAFLUSH, &original_stdin_termios);
}

static void restore_stdout_termios(void)
{
    tcsetattr(s_out_fd, TCSAFLUSH, &original_stdout_termios);
}

void unix_uart_restore(void)
{
    restore_stdin_termios();
    restore_stdout_termios();
    dup2(s_out_fd, STDOUT_FILENO);
}

void unix_uart_enable(void)
{
    struct termios termios;

    s_in_fd = dup(STDIN_FILENO);
    s_out_fd = dup(STDOUT_FILENO);
    dup2(STDERR_FILENO, STDOUT_FILENO);

    signal(SIGPIPE, SIG_DFL);

    if (isatty(s_in_fd)) {
        tcgetattr(s_in_fd, &original_stdin_termios);
        atexit(&restore_stdin_termios);
    }

    if (isatty(s_out_fd)) {
        tcgetattr(s_out_fd, &original_stdout_termios);
        atexit(&restore_stdout_termios);
    }

    if (isatty(s_in_fd)) {
        // get current configuration
        if(tcgetattr(s_in_fd, &termios) != 0) {
            perror("tcgetattr");
            goto exit;
        }
        // Set up the termios settings for raw mode. This turns
        // off input/output processing, line processing, and character processing.
        cfmakeraw(&termios);
        // Set up our cflags for local use. Turn on hangup-on-close.
        termios.c_cflag |= HUPCL | CREAD | CLOCAL;
        // "Minimum number of characters for noncanonical read"
        termios.c_cc[VMIN] = 1;
        // "Timeout in deciseconds for noncanonical read"
        termios.c_cc[VTIME] = 0;
        // configure baud rate
        if (cfsetispeed(&termios, B115200) != 0) {
            perror("cfsetispeed");
            goto exit;
        }
        // set configuration
        if (tcsetattr(s_in_fd, TCSANOW, &termios) != 0) {
            perror("tcsetattr");
            goto exit;
        }
    }

    if (isatty(s_out_fd)) {
        // get current configuration
        if (tcgetattr(s_out_fd, &termios) != 0) {
            perror("tcgetattr");
            goto exit;
        }
        // Set up the termios settings for raw mode. This turns
        // off input/output processing, line processing, and character processing.
        cfmakeraw(&termios);
        // Absolutely obliterate all output processing.
        termios.c_oflag = 0;
        // Set up our cflags for local use. Turn on hangup-on-close.
        termios.c_cflag |= HUPCL | CREAD | CLOCAL;
        // configure baud rate
        if (cfsetospeed(&termios, B115200) != 0) {
            perror("cfsetospeed");
            goto exit;
        }
        // set configuration
        if (tcsetattr(s_out_fd, TCSANOW, &termios) != 0) {
            perror("tcsetattr");
            goto exit;
        }
    }

    return;

exit:
    close(s_in_fd);
    close(s_out_fd);
}

void unix_uart_disable(void)
{
    close(s_in_fd);
    close(s_out_fd);
}

void unix_uart_send(const uint8_t *buf, uint16_t buf_len)
{
    if (s_write_length != 0) {
        goto exit; // uart was busy
    }

    s_write_buffer = buf;
    s_write_length = buf_len;

exit:
    return;
}

void unix_uart_update_fd_set(fd_set *read_fd_set,
                             fd_set *write_fd_set,
                             fd_set *error_fd_set,
                             int *max_fd)
{
    if (read_fd_set != NULL) {
        FD_SET(s_in_fd, read_fd_set);
        if (error_fd_set != NULL) {
            FD_SET(s_in_fd, error_fd_set);
        }
        if (max_fd != NULL && *max_fd < s_in_fd) {
            *max_fd = s_in_fd;
        }
    }
    if ((write_fd_set != NULL) && (s_write_length > 0)) {
        FD_SET(s_out_fd, write_fd_set);
        if (error_fd_set != NULL) {
            FD_SET(s_out_fd, error_fd_set);
        }
        if (max_fd != NULL && *max_fd < s_out_fd) {
            *max_fd = s_out_fd;
        }
    }
}

void unix_uart_process(void)
{
    ssize_t rval;
    const int     error_flags = POLLERR | POLLNVAL | POLLHUP;
    struct pollfd pollfd[]    = {
        {s_in_fd, POLLIN | error_flags, 0},
        {s_out_fd, POLLOUT | error_flags, 0},
    };

    errno = 0;
    rval = poll(pollfd, sizeof(pollfd) / sizeof(*pollfd), 0);

    if (rval < 0) {
        perror("poll");
        exit(EXIT_FAILURE);
    }

    if (rval > 0) {
        if ((pollfd[0].revents & error_flags) != 0) {
            perror("s_in_fd");
            exit(EXIT_FAILURE);
        }
        if ((pollfd[1].revents & error_flags) != 0) {
            perror("s_out_fd");
            exit(EXIT_FAILURE);
        }
        if (pollfd[0].revents & POLLIN) {
            rval = read(s_in_fd, s_receive_buffer, sizeof(s_receive_buffer));
            if (rval <= 0) {
                perror("read");
                exit(EXIT_FAILURE);
            }
            platform_uart_received(s_receive_buffer, (uint16_t)rval);
        }

        if ((s_write_length > 0) && (pollfd[1].revents & POLLOUT)) {
            rval = write(s_out_fd, s_write_buffer, s_write_length);
            if (rval <= 0) {
                perror("write");
                exit(EXIT_FAILURE);
            }
            s_write_buffer += (uint16_t)rval;
            s_write_length -= (uint16_t)rval;
            if (s_write_length == 0) {
                platform_uart_send_done();
            }
        }
    }
}

void platform_uart_init(void)
{
    unix_uart_enable();
}

void platform_uart_send(const uint8_t *buf, uint16_t buf_len)
{
    unix_uart_send(buf, buf_len);
}
