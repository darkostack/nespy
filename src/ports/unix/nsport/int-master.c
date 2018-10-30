#include "ns/contiki.h"
#include "ns/sys/int-master.h"
#include <stdbool.h>

void int_master_enable(void)
{
}

int_master_status_t int_master_read_and_disable(void)
{
    return 0;
}

void int_master_status_set(int_master_status_t status)
{
    (void)status;
}

bool int_master_is_enable(void)
{
    return 0;
}
