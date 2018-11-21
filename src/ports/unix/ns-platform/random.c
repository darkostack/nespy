#include <assert.h>
#include <stdio.h>
#include "ns-platform/platform-unix.h"
#include "ns/include/platform/random.h"

static uint32_t state = 1;

void
plat_random_init(void)
{
    state = (uint32_t)time(NULL) + (3600 * node_id);
}

uint32_t
ns_plat_random_get(void)
{
    uint32_t mlcg, p, q;
    uint64_t tmpstate;
    tmpstate = (uint64_t)33614 * (uint64_t)state;
    q = tmpstate & 0xffffffff;
    q = q >> 1;
    p = tmpstate >> 32;
    mlcg = p + q;
    if (mlcg & 0x80000000) {
        mlcg &= 0x7fffffff;
        mlcg++;
    }
    state = mlcg;
    return mlcg;
}

ns_error_t
ns_plat_random_get_true(uint8_t *output, uint16_t output_length)
{
    ns_error_t error = NS_ERROR_NONE;
    if (!(output && output_length)) {
        error = NS_ERROR_INVALID_ARGS;
        goto exit;
    }
    for (uint16_t len = 0; len < output_length; len++) {
        output[len] = (uint8_t)ns_plat_random_get();
    }
exit:
    return error;
}
