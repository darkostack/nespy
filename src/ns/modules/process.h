#ifndef NS_MODULES_PROCESS_H_
#define NS_MODULES_PROCESS_H_

#include "ns/contiki.h"

#define NS_THREAD_DEPTH         2
#define NS_INVALID_THREAD_ID    0xff

#define NS_PROCESS_THREAD(id)                                       \
PROCESS_THREAD(thread ## id, ev, data) {                            \
    PROCESS_BEGIN();                                                \
    while (1) {                                                     \
        PROCESS_WAIT_EVENT_UNTIL((ev == PROCESS_EVENT_POLL) ||      \
                                 (ev == PROCESS_EVENT_EXIT));       \
        if (ev == PROCESS_EVENT_POLL) {                             \
            if (thread_container.evid[id] == PROCESS_EVENT_INIT) {  \
                ns_thread_obj_t *self = &thread_container.obj[id];  \
                mp_obj_t cbev = mp_obj_new_int_from_uint(self->ev); \
                mp_obj_t cbdata = mp_obj_new_int(*(int *)data);     \
                mp_call_function_2(self->cb, cbev, cbdata);         \
            }                                                       \
        } else if (ev == PROCESS_EVENT_EXIT) {                      \
            thread_container.evid[id] = PROCESS_EVENT_NONE;         \
            thread_container.nthread--;                             \
            PROCESS_EXIT();                                         \
        }                                                           \
    }                                                               \
    PROCESS_END();                                                  \
}

typedef int ns_thread_id_t;

typedef struct _ns_thread_obj_t {
    mp_obj_base_t base;
    mp_obj_t cb;
    ns_thread_id_t id;
    process_event_t ev;
} ns_thread_obj_t;

typedef struct _ns_thread_container_t {
    ns_thread_obj_t obj[NS_THREAD_DEPTH];
    process_event_t evid[NS_THREAD_DEPTH];
    uint8_t nthread;
} ns_thread_container_t;

typedef struct _ns_process_base_obj_t {
    mp_obj_base_t base;
    struct process *list;
} ns_process_base_obj_t;


extern const mp_obj_type_t ns_process_type;
extern const mp_obj_type_t ns_thread_type;

#endif // NS_MODULES_PROCESS_H_
