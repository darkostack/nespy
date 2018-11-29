#include "ns/include/error.h"
#include "ns/sys/core/common/instance.h"
#include "ns/sys/core/common/random.h"
#include "ns/sys/core/mac/mac_frame.h"
#include <string.h>

// --- MAC extended address functions
void
mac_ext_addr_gen_random(ext_addr_t *ext_addr)
{
    random_fill_buffer(ext_addr->m8, sizeof(ext_addr_t));
    mac_ext_addr_set_group(ext_addr, false);
    mac_ext_addr_set_local(ext_addr, true);
}

bool
mac_ext_addr_is_group(ext_addr_t *ext_addr)
{
    return ((ext_addr->m8[0] & MAC_EXT_ADDR_GROUP_FLAG) != 0);
}

void
mac_ext_addr_set_group(ext_addr_t *ext_addr, bool group)
{
    if (group) {
        ext_addr->m8[0] |= MAC_EXT_ADDR_GROUP_FLAG;
    } else {
        ext_addr->m8[0] &= ~MAC_EXT_ADDR_GROUP_FLAG;
    }
}

void
mac_ext_addr_toggle_group(ext_addr_t *ext_addr)
{
    ext_addr->m8[0] ^= MAC_EXT_ADDR_GROUP_FLAG;
}

bool
mac_ext_addr_is_local(ext_addr_t *ext_addr)
{
    return ((ext_addr->m8[0] & MAC_EXT_ADDR_LOCAL_FLAG) != 0);
}

void
mac_ext_addr_set_local(ext_addr_t *ext_addr, bool local)
{
    if (local) {
        ext_addr->m8[0] |= MAC_EXT_ADDR_LOCAL_FLAG;
    } else {
        ext_addr->m8[0] &= ~MAC_EXT_ADDR_LOCAL_FLAG;
    }
}

void
mac_ext_addr_toggle_local(ext_addr_t *ext_addr)
{
    ext_addr->m8[0] ^= MAC_EXT_ADDR_LOCAL_FLAG;
}

bool
mac_ext_addr_is_equal(ext_addr_t *ext_addr1, ext_addr_t *ext_addr2)
{
    return (memcmp(ext_addr1->m8, ext_addr2->m8, sizeof(ext_addr_t)) == 0);
}
