#ifndef NS_CLI_UART_H_
#define NS_CLI_UART_H_

#include <stdarg.h>
#include <stdint.h>
#include "core/core-config.h"

enum {
    CLI_UART_RX_BUF_SIZE = NS_CONFIG_CLI_RX_BUFFER_SIZE,
    CLI_UART_TX_BUF_SIZE = NS_CONFIG_CLI_TX_BUFFER_SIZE,
    CLI_UART_CLI_MAX_LINE_LEN = NS_CONFIG_CLI_MAX_LINE_LEN,
};

void
cli_uart_init(void);

void
cli_uart_output_bytes(const uint8_t *bytes, uint8_t len);

void
cli_uart_output(const char *buf, uint16_t buf_len);

void
cli_uart_output_format(const char *format, ...);

void
cli_uart_receive_task(const uint8_t *buf, uint16_t buf_len);

void
cli_uart_send_done_task(void);

#endif // NS_CLI_UART_H_
