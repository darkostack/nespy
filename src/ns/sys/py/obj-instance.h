#ifndef NS_PY_OBJ_INSTANCE_H_
#define NS_PY_OBJ_INSTANCE_H_

#include "ns/include/instance.h"

typedef struct _py_instance_obj_t {
    mp_obj_base_t base;
    ns_instance_t instance; // instance pointer
} py_instance_obj_t;

#endif // NS_PY_OBJ_INSTANCE_H_
