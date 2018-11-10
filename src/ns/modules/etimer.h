#ifndef NS_MODULES_ETIMER_H_
#define NS_MODULES_ETIMER_H_

#define ETIMER_OBJ_ALL_NUM 2

typedef struct _ns_etimer_obj_t {
    mp_obj_base_t base;
    struct etimer timer;
    clock_time_t period;
} ns_etimer_obj_t;

#endif // NS_MODULES_ETIMER_H_
