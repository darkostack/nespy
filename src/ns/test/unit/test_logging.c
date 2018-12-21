#include "ns/sys/core/core-config.h"
#include "ns/include/platform/random.h"
#include "ns/sys/core/common/instance.h"

#define PRINT_LOGGING 0

static void
print_logging(void)
{
#if PRINT_LOGGING
    ns_log_crit_api("print test 0x%08x", 0xdeadbeef);

    uint8_t dump_buffer[128];

    for (unsigned i = 0; i < sizeof(dump_buffer); i++)
    {
        dump_buffer[i] = (uint8_t)ns_plat_random_get();
    }

    ns_dump_crit_mac("test buffer", dump_buffer, sizeof(dump_buffer));
#endif
}

void
test_logging(void)
{
    print_logging();
}
