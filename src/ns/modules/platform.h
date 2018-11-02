#ifndef NS_MODULES_PLATFORM_H_
#define NS_MODULES_PLATFORM_H_

void platform_uart_init(void);
void platform_uart_received(const uint8_t *buf, uint16_t buf_len);
void platform_uart_send(const uint8_t *buf, uint16_t buf_len);
void platform_uart_send_done(void);

#endif // NS_MODULES_PLATFORM_H_
