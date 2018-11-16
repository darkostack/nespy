#include "ns/include/instance.h"
#include "ns/sys/core/common/instance.h"

ns_instance_t ns_instance_init(void)
{
    ns_instance_t inst = (ns_instance_t)instance_init();
    return inst;
}

ns_instance_t ns_instance_get(void)
{
    ns_instance_t inst = (ns_instance_t)instance_get();
    return inst;
}

bool ns_instance_is_initialized(ns_instance_t instance)
{
    return instance_is_initialized((instance_t *)instance);
}
