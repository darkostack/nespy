#include "ns/contiki.h"
#include "ns/lib/random.h"
#include "port_unix.h"

static uint32_t state = 1;

void random_init(unsigned short seed)
{
    state = (uint32_t)time(NULL) + (3600 * seed);
}

unsigned short random_rand(void)
{
    uint32_t mlcg, p, q;
    uint64_t tmpstate;

    tmpstate = (uint64_t)33614 * (uint64_t)state;
    q        = tmpstate & 0xffffffff;
    q        = q >> 1;
    p        = tmpstate >> 32;
    mlcg     = p + q;

    if (mlcg & 0x80000000)
    {
        mlcg &= 0x7fffffff;
        mlcg++;
    }

    state = mlcg;

    return mlcg;
}
