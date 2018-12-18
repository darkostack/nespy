#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include "ns/include/platform/uart.h"
#include "platform-unix.h"

static uint8_t        s_receive_buffer[128];
static const uint8_t *s_write_buffer;
static uint16_t       s_write_length;
static int            s_in_fd;
static int            s_out_fd;
static struct termios s_original_stdin_termios;
static struct termios s_original_stdout_termios;

static void
restore_stdin_termios(void)
{
    tcsetattr(s_in_fd, TCSAFLUSH, &s_original_stdin_termios);
}

static void
restore_stdout_termios(void)
{
    tcsetattr(s_out_fd, TCSAFLUSH, &s_original_stdout_termios);
}

void
plat_uart_restore(void)
{
    restore_stdin_termios();
    restore_stdout_termios();
    dup2(s_out_fd, STDOUT_FILENO);
}

ns_error_t
ns_plat_uart_enable(void)
{
    ns_error_t error = NS_ERROR_NONE;
    struct termios termios;

    s_in_fd = dup(STDIN_FILENO);
    s_out_fd = dup(STDOUT_FILENO);
    dup2(STDERR_FILENO, STDOUT_FILENO);

    // we need this signal to make sure that this process terminates properly
    signal(SIGPIPE, SIG_DFL);

    if (isatty(s_in_fd)) {
        tcgetattr(s_in_fd, &s_original_stdin_termios);
        atexit(&restore_stdin_termios);
    }

    if (isatty(s_out_fd)) {
        tcgetattr(s_out_fd, &s_original_stdout_termios);
        atexit(&restore_stdout_termios);
    }

    if (isatty(s_in_fd))
    {
        // get current configuration
        if (tcgetattr(s_in_fd, &termios) != 0) {
            perror("tcgetattr");
            error = NS_ERROR_GENERIC;
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
        if (cfsetispeed(&termios, NS_THREAD_UNIX_UART_BAUDRATE) != 0) {
            perror("cfsetispeed");
            error = NS_ERROR_GENERIC;
            goto exit;
        }

        // set configuration
        if (tcsetattr(s_in_fd, TCSANOW, &termios) != 0) {
            perror("tcsetattr");
            error = NS_ERROR_GENERIC;
            goto exit;
        }
    }

    if (isatty(s_out_fd))
    {
        // get current configuration
        if (tcgetattr(s_out_fd, &termios) != 0) {
            perror("tcgetattr");
            error = NS_ERROR_GENERIC;
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
        if (cfsetospeed(&termios, NS_THREAD_UNIX_UART_BAUDRATE) != 0) {
            perror("cfsetospeed");
            error = NS_ERROR_GENERIC;
            goto exit;
        }

        // set configuration
        if (tcsetattr(s_out_fd, TCSANOW, &termios) != 0) {
            perror("tcsetattr");
            error = NS_ERROR_GENERIC;
            goto exit;
        }
    }

    return error;

exit:
    close(s_in_fd);
    close(s_out_fd);
    return error;
}

ns_error_t
ns_plat_uart_disable(void)
{
    close(s_in_fd);
    close(s_out_fd);
    return NS_ERROR_NONE;
}

ns_error_t
ns_plat_uart_send(const uint8_t *buf, uint16_t buf_length)
{
    ns_error_t error = NS_ERROR_NONE;
    if (s_write_length != 0) {
        error = NS_ERROR_BUSY;
        goto exit;
    }
    s_write_buffer = buf;
    s_write_length = buf_length;
exit:
    return error;
}

void
plat_uart_update_fd_set(fd_set *read_fd, fd_set *write_fd, fd_set *error_fd, int *max_fd)
{
    if (read_fd != NULL) {
        FD_SET(s_in_fd, read_fd);
        if (error_fd != NULL) {
            FD_SET(s_in_fd, error_fd);
        }
        if (max_fd != NULL && *max_fd < s_in_fd) {
            *max_fd = s_in_fd;
        }
    }
    if (write_fd != NULL) {
        FD_SET(s_out_fd, write_fd);
        if (error_fd != NULL) {
            FD_SET(s_out_fd, error_fd);
        }
        if (max_fd != NULL && *max_fd < s_out_fd) {
            *max_fd = s_out_fd;
        }
    }
}

void
plat_uart_process(void)
{
    ssize_t rval;
    const int error_flags = POLLERR | POLLNVAL | POLLHUP;
    struct pollfd pollfd[] = {
        { s_in_fd, POLLIN | error_flags, 0 },
        { s_out_fd, POLLOUT | error_flags, 0 },
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
            ns_plat_uart_received(s_receive_buffer, (uint16_t)rval);
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
                ns_plat_uart_send_done();
            }
        }
    }
}
