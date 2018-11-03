#include "ns/modules/cli/cli.h"
#include "ns/modules/cli/cli-uart.h"
#include "ns/modules/nstd.h"
#include "ns/modules/platform.h"

static char s_rx_buffer[const_rx_buf_size];
static uint16_t s_rx_length;
static char s_tx_buffer[const_tx_buf_size];
static uint16_t s_tx_head;
static uint16_t s_tx_length;
static uint16_t s_send_length;

static const char s_command_prompt[] = {'>', '>', '>', ' '};
static const char s_erase_string[] = {'\b', ' ', '\b'};
static const char CRNL[] = {'\r', '\n'};

static void process_command(void);
static void output_format(const char *fmt, va_list ap);
static int output(const char *buf, uint16_t buf_len);
static void send(void);

void platform_uart_received(const uint8_t *buf, uint16_t buf_len)
{
    cli_uart_receive_task(buf, buf_len);
}

void platform_uart_send_done(void)
{
    cli_uart_send_done_task();
}

void cli_uart_init(void)
{
    cli_commands_init();
    platform_uart_init();
}

void cli_uart_output_bytes(const uint8_t *bytes, uint8_t len)
{
    cli_output_bytes(bytes, len);
}

void cli_uart_output_format(const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    output_format(format, ap);
    va_end(ap);
}

void cli_uart_receive_task(const uint8_t *buf, uint16_t buf_len)
{
    const uint8_t *end;

    end = buf + buf_len;

    for (; buf < end; buf++) {
        switch (*buf) {
        case '\r':
        case '\n':
            output(CRNL, sizeof(CRNL));
            if (s_rx_length > 0) {
                s_rx_buffer[s_rx_length] = '\0';
                process_command();
            }

            output(s_command_prompt, sizeof(s_command_prompt));
            break;
#if defined(UNIX)
        case 0x04: // ASCII for CTRL-D
            exit(EXIT_SUCCESS);
            break;
#endif
        case '\b':
        case 127:
            if (s_rx_length > 0) {
                output(s_erase_string, sizeof(s_erase_string));
                s_rx_buffer[--s_rx_length] = '\0';
            }
            break;
        default:
            if (s_rx_length < const_rx_buf_size) {
                output((const char *)buf, 1);
                s_rx_buffer[s_rx_length++] = (char)*buf;
            }
            break;
        }
    }
}

void cli_uart_send_done_task(void)
{
    s_tx_head = (s_tx_head + s_send_length) % const_tx_buf_size;
    s_tx_length -= s_send_length;
    s_send_length = 0;

    send();
}

static void process_command(void)
{
    if (s_rx_buffer[s_rx_length - 1] == '\n') {
        s_rx_buffer[--s_rx_length] = '\0';
    }
    if (s_rx_buffer[s_rx_length - 1] == '\r') {
        s_rx_buffer[--s_rx_length] = '\0';
    }

    cli_process_line(s_rx_buffer, s_rx_length);

    s_rx_length = 0;
}

static void output_format(const char *fmt, va_list ap)
{
    char buf[const_cli_max_len];
    vsnprintf(buf, sizeof(buf), fmt, ap);
    output(buf, (uint16_t)ns_strlen(buf));
}

static int output(const char *buf, uint16_t buf_len)
{
    uint16_t remaining = const_tx_buf_size - s_tx_length;
    uint16_t tail;
    if (buf_len > remaining) {
        buf_len = remaining;
    }
    for (int i = 0; i < buf_len; i++) {
        tail = (s_tx_head + s_tx_length) % const_tx_buf_size;
        s_tx_buffer[tail] = *buf++;
        s_tx_length++;
    }

    send();

    return buf_len;
}

static void send(void)
{
    if (s_send_length != 0) {
        return;
    }
    if (s_tx_length > const_tx_buf_size - s_tx_head) {
        s_send_length = const_tx_buf_size - s_tx_head;
    } else {
        s_send_length = s_tx_length;
    }
    if (s_send_length > 0) {
        platform_uart_send((uint8_t *)&s_tx_buffer[s_tx_head], s_send_length);
    }
}
