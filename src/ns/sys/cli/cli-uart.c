#include "ns/sys/cli/cli.h"
#include "ns/sys/cli/cli-uart.h"
#include "ns/include/platform/uart.h"
#include "ns/include/nstd.h"

static char cli_rx_buffer[CLI_UART_RX_BUF_SIZE];
static uint16_t cli_rx_length;
static char cli_tx_buffer[CLI_UART_TX_BUF_SIZE];
static uint16_t cli_tx_head;
static uint16_t cli_tx_length;
static uint16_t cli_send_length;

static const char cli_command_prompt[] = {'>', '>', '>', ' '};
static const char cli_erase_str[] = {'\b', ' ', '\b'};
static const char CRNL[] = {'\r', '\n'};

static void process_command(void);
static void output_format(const char *fmt, va_list ap);
static int output(const char *buf, uint16_t buf_len);
static void send(void);

void ns_plat_uart_send_done(void)
{
    cli_uart_send_done_task();
}

void ns_plat_uart_received(const uint8_t *buf, uint16_t buf_length)
{
    cli_uart_receive_task(buf, buf_length);
}

void cli_uart_init(void)
{
    ns_plat_uart_enable();
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
            if (cli_rx_length > 0) {
                cli_rx_buffer[cli_rx_length] = '\0';
                process_command();
            }

            output(cli_command_prompt, sizeof(cli_command_prompt));
            break;
#if defined(UNIX)
        case 0x04: // ASCII for CTRL-D
            exit(EXIT_SUCCESS);
            break;
#endif
        case '\b':
        case 127:
            if (cli_rx_length > 0) {
                output(cli_erase_str, sizeof(cli_erase_str));
                cli_rx_buffer[--cli_rx_length] = '\0';
            }
            break;
        default:
            if (cli_rx_length < CLI_UART_RX_BUF_SIZE) {
                output((const char *)buf, 1);
                cli_rx_buffer[cli_rx_length++] = (char)*buf;
            }
            break;
        }
    }
}

void cli_uart_send_done_task(void)
{
    cli_tx_head = (cli_tx_head + cli_send_length) % CLI_UART_TX_BUF_SIZE;
    cli_tx_length -= cli_send_length;
    cli_send_length = 0;
    send();
}

static void process_command(void)
{
    if (cli_rx_buffer[cli_rx_length - 1] == '\n') {
        cli_rx_buffer[--cli_rx_length] = '\0';
    }
    if (cli_rx_buffer[cli_rx_length - 1] == '\r') {
        cli_rx_buffer[--cli_rx_length] = '\0';
    }
    cli_process_line(cli_rx_buffer, cli_rx_length);
    cli_rx_length = 0;
}

static void output_format(const char *fmt, va_list ap)
{
    char buf[CLI_UART_CLI_MAX_LINE_LEN];
    vsnprintf(buf, sizeof(buf), fmt, ap);
    output(buf, (uint16_t)ns_strlen(buf));
}

static int output(const char *buf, uint16_t buf_len)
{
    uint16_t remaining = CLI_UART_TX_BUF_SIZE - cli_tx_length;
    uint16_t tail;
    if (buf_len > remaining) {
        buf_len = remaining;
    }
    for (int i = 0; i < buf_len; i++) {
        tail = (cli_tx_head + cli_tx_length) % CLI_UART_TX_BUF_SIZE;
        cli_tx_buffer[tail] = *buf++;
        cli_tx_length++;
    }
    send();
    return buf_len;
}

static void send(void)
{
    if (cli_send_length != 0) {
        return;
    }
    if (cli_tx_length > CLI_UART_TX_BUF_SIZE - cli_tx_head) {
        cli_send_length = CLI_UART_TX_BUF_SIZE - cli_tx_head;
    } else {
        cli_send_length = cli_tx_length;
    }
    if (cli_send_length > 0) {
        ns_plat_uart_send((uint8_t *)&cli_tx_buffer[cli_tx_head], cli_send_length);
    }
}
