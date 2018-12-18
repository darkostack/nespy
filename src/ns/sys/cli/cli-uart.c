#include "ns/sys/cli/cli.h"
#include "ns/sys/cli/cli-uart.h"
#include "ns/include/platform/uart.h"
#include "ns/include/nstd.h"

static char     s_cli_rx_buffer[CLI_UART_RX_BUF_SIZE];
static uint16_t s_cli_rx_length;
static char     s_cli_tx_buffer[CLI_UART_TX_BUF_SIZE];
static uint16_t s_cli_tx_head;
static uint16_t s_cli_tx_length;
static uint16_t s_cli_send_length;

static const char s_cli_command_prompt[] = {'>', '>', '>', ' '};
static const char s_cli_erase_str[] = {'\b', ' ', '\b'};
static const char s_crnl[] = {'\r', '\n'};

static void
process_command(void);

static void
output_format(const char *fmt, va_list ap);

static int
output(const char *buf, uint16_t buf_len);

static void
send(void);

void
ns_plat_uart_send_done(void)
{
    cli_uart_send_done_task();
}

void
ns_plat_uart_received(const uint8_t *buf, uint16_t buf_length)
{
    cli_uart_receive_task(buf, buf_length);
}

void
cli_uart_init(void)
{
    ns_plat_uart_enable();
}

void
cli_uart_output_bytes(const uint8_t *bytes, uint8_t len)
{
    cli_output_bytes(bytes, len);
}

void
cli_uart_output_format(const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    output_format(format, ap);
    va_end(ap);
}

void
cli_uart_receive_task(const uint8_t *buf, uint16_t buf_len)
{
    const uint8_t *end;

    end = buf + buf_len;

    for (; buf < end; buf++) {
        switch (*buf) {
        case '\r':
        case '\n':
            output(s_crnl, sizeof(s_crnl));
            if (s_cli_rx_length > 0) {
                s_cli_rx_buffer[s_cli_rx_length] = '\0';
                process_command();
            }

            output(s_cli_command_prompt, sizeof(s_cli_command_prompt));
            break;
#if defined(UNIX)
        case 0x04: // ASCII for CTRL-D
            exit(EXIT_SUCCESS);
            break;
#endif
        case '\b':
        case 127:
            if (s_cli_rx_length > 0) {
                output(s_cli_erase_str, sizeof(s_cli_erase_str));
                s_cli_rx_buffer[--s_cli_rx_length] = '\0';
            }
            break;
        default:
            if (s_cli_rx_length < CLI_UART_RX_BUF_SIZE) {
                output((const char *)buf, 1);
                s_cli_rx_buffer[s_cli_rx_length++] = (char)*buf;
            }
            break;
        }
    }
}

void
cli_uart_send_done_task(void)
{
    s_cli_tx_head = (s_cli_tx_head + s_cli_send_length) % CLI_UART_TX_BUF_SIZE;
    s_cli_tx_length -= s_cli_send_length;
    s_cli_send_length = 0;
    send();
}

static void
process_command(void)
{
    if (s_cli_rx_buffer[s_cli_rx_length - 1] == '\n') {
        s_cli_rx_buffer[--s_cli_rx_length] = '\0';
    }
    if (s_cli_rx_buffer[s_cli_rx_length - 1] == '\r') {
        s_cli_rx_buffer[--s_cli_rx_length] = '\0';
    }
    cli_process_line(s_cli_rx_buffer, s_cli_rx_length);
    s_cli_rx_length = 0;
}

static void
output_format(const char *fmt, va_list ap)
{
    char buf[CLI_UART_CLI_MAX_LINE_LEN];
    vsnprintf(buf, sizeof(buf), fmt, ap);
    output(buf, (uint16_t)ns_strlen(buf));
}

static int
output(const char *buf, uint16_t buf_len)
{
    uint16_t remaining = CLI_UART_TX_BUF_SIZE - s_cli_tx_length;
    uint16_t tail;
    if (buf_len > remaining) {
        buf_len = remaining;
    }
    for (int i = 0; i < buf_len; i++) {
        tail = (s_cli_tx_head + s_cli_tx_length) % CLI_UART_TX_BUF_SIZE;
        s_cli_tx_buffer[tail] = *buf++;
        s_cli_tx_length++;
    }
    send();
    return buf_len;
}

static void
send(void)
{
    if (s_cli_send_length != 0) {
        return;
    }
    if (s_cli_tx_length > CLI_UART_TX_BUF_SIZE - s_cli_tx_head) {
        s_cli_send_length = CLI_UART_TX_BUF_SIZE - s_cli_tx_head;
    } else {
        s_cli_send_length = s_cli_tx_length;
    }
    if (s_cli_send_length > 0) {
        ns_plat_uart_send((uint8_t *)&s_cli_tx_buffer[s_cli_tx_head], s_cli_send_length);
    }
}
