#include "ns/contiki.h"
#include "ns/sys/platform.h"
#include "ns/sys/node-id.h"
#include "ns/lib/random.h"

void platform_init_stage_one(void)
{
    // reserved for boot up init sequence
}

void platform_init_stage_two(void)
{
    // platform init for networking
    random_init(node_id);
}

void platform_init_stage_three(void)
{
    // low-level driver and sensor (optional)
}
