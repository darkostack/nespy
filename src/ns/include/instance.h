#ifndef NS_INSTANCE_H_
#define NS_INSTANCE_H_

#include <stdbool.h>
#include <stdint.h>

typedef void *ns_instance_t;

ns_instance_t
ns_instance_init(void);

ns_instance_t
ns_instance_get(void);

bool
ns_instance_is_initialized(ns_instance_t instance);

#endif // NS_INSTANCE_H_
