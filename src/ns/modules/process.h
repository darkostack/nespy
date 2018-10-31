#ifndef NS_MODULES_PROCESS_H_
#define NS_MODULES_PROCESS_H_

#include "ns/contiki.h"

#define NS_THREAD_DEPTH         2
#define NS_INVALID_THREAD_ID    0xff

#define NS_PROCESS_THREAD(id)                                                 \
PROCESS_THREAD(thread ## id, ev, data) {                                      \
    PROCESS_BEGIN();                                                          \
    while (1) {                                                               \
        PROCESS_WAIT_EVENT_UNTIL((ev == PROCESS_EVENT_POLL) ||                \
                                 (ev == PROCESS_EVENT_EXIT));                 \
        if (ev == PROCESS_EVENT_POLL) {                                       \
            if ((process_event_t)mp_obj_get_int(thread_container.evid[id]) == \
                PROCESS_EVENT_INIT) {                                         \
                ns_thread_obj_t *self = &thread_container.obj[id];            \
                mp_obj_t cbev = self->ev;                                     \
                mp_obj_t cbdata = self->data;                                 \
                mp_call_function_2(self->cb, cbev, cbdata);                   \
            }                                                                 \
        } else if (ev == PROCESS_EVENT_EXIT) {                                \
            thread_container.evid[id] = mp_obj_new_int_from_uint(PROCESS_EVENT_NONE); \
            thread_container.nthread--;                                       \
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
    mp_obj_t ev;
    mp_obj_t data;
} ns_thread_obj_t;

typedef struct _ns_thread_container_t {
    ns_thread_obj_t obj[NS_THREAD_DEPTH];
    mp_obj_t evid[NS_THREAD_DEPTH];
    uint8_t nthread;
} ns_thread_container_t;

typedef struct _ns_process_base_obj_t {
    mp_obj_base_t base;
    struct process *list;
} ns_process_base_obj_t;

#endif // NS_MODULES_PROCESS_H_
