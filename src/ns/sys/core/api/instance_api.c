#include "ns/include/instance.h"
#include "ns/sys/core/common/instance.h"
#include "ns/sys/cli/cli-uart.h"
#include "ns/sys/cli/cli.h"

ns_instance_t
ns_instance_init(void)
{
    ns_instance_t instance = (ns_instance_t)instance_init();

    // --- initialize nespy module
    cli_uart_init();
    cli_commands_init();

    return instance;
}

ns_instance_t
ns_instance_get(void)
{
    return (ns_instance_t)instance_get();
}

bool
ns_instance_is_initialized(ns_instance_t instance)
{
    instance_t *inst = (instance_t *)instance;
    return inst->is_initialized;
}
