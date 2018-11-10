#ifndef NS_MODULES_PROCESS_H_
#define NS_MODULES_PROCESS_H_

#include "ns/contiki.h"

#define THREAD_OBJ_ALL_NUM 10

#define NS_PROCESS_THREAD(n)                                                  \
PROCESS_THREAD(thread ## n, ev, data) {                                       \
    PROCESS_BEGIN();                                                          \
    while (1) {                                                               \
        PROCESS_WAIT_EVENT();                                                 \
        ns_thread_obj_t *self = &thread_obj_all.thread[n];                    \
        if (ev != PROCESS_EVENT_EXIT) {                                       \
            if (self->is_used) {                                              \
                mp_obj_t event = mp_obj_new_int_from_uint(ev);                \
                mp_call_function_2(self->cb, event, self->data);              \
                self->data = mp_const_none;                                   \
            }                                                                 \
        } else {                                                              \
            self->cb = mp_const_none;                                         \
            self->id = 0;                                                     \
            self->data = mp_const_none;                                       \
            self->is_used = false;                                            \
            thread_obj_all.thread[n] = *self;                                 \
            thread_obj_all.remain++;                                          \
            PROCESS_EXIT();                                                   \
        }                                                                     \
    }                                                                         \
    PROCESS_END();                                                            \
}

typedef int ns_thread_id_t;

typedef struct _ns_thread_obj_t {
    mp_obj_base_t base;
    mp_obj_t cb;
    ns_thread_id_t id;
    mp_obj_t data;
    bool is_used;
} ns_thread_obj_t;

typedef struct _ns_thread_obj_all_t {
    ns_thread_obj_t thread[THREAD_OBJ_ALL_NUM];
    uint8_t remain;
} ns_thread_obj_all_t;

typedef struct _ns_process_base_obj_t {
    mp_obj_base_t base;
} ns_process_base_obj_t;

#endif // NS_MODULES_PROCESS_H_
