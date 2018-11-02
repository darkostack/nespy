#include "ns/contiki.h"
#include "ns/sys/platform.h"
#include "ns/modules/cli/cli-uart.h"
#include "ns/modules/nstd.h"

void platform_init_stage_one(void)
{
    cli_uart_init();
}

void platform_init_stage_two(void)
{
}

void platform_init_stage_three(void)
{
}
