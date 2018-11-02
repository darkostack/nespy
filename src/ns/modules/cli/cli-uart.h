#ifndef NS_MODULES_CLI_UART_H_
#define NS_MODULES_CLI_UART_H_

#include <stdarg.h>
#include <stdint.h>

enum {
    const_rx_buf_size = 512,
    const_tx_buf_size = 1024,
    const_cli_max_len = 128,
};

void cli_uart_init(void);
void cli_uart_output_bytes(const uint8_t *bytes, uint8_t len);
void cli_uart_output(const char *buf, uint16_t buf_len);
void cli_uart_output_format(const char *format, ...);
void cli_uart_receive_task(const uint8_t *buf, uint16_t buf_len);
void cli_uart_send_done_task(void);

#endif // NS_MODULES_CLI_UART_H_
