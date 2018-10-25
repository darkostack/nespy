#include "contiki.h"
#include "ns/lib/random.h"
#include "ns/sys/clock.h"

void random_init(unsigned short seed)
{
    (void)seed;
}

unsigned short random_rand(void)
{
    return 0;
}
