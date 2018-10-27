#include "ns/contiki.h"
#include "ns/dev/slip.h"

void slip_arch_writeb(unsigned char c)
{
    (void) c;
}

void sli_arch_init()
{
    return;
}
