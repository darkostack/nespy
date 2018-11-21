#ifndef NS_PLATFORM_UART_H_
#define NS_PLATFORM_UART_H_

#include <stdint.h>
#include "ns/include/error.h"

ns_error_t
ns_plat_uart_enable(void);

ns_error_t
ns_plat_uart_disable(void);

ns_error_t
ns_plat_uart_send(const uint8_t *buf, uint16_t buf_length);

extern void
ns_plat_uart_send_done(void);

extern void
ns_plat_uart_received(const uint8_t *buf, uint16_t buf_length);

#endif // NS_PLATFORM_UART_H_
