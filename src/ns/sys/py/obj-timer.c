#include "py/nlr.h"
#include "py/runtime.h"
#include "ns/sys/py/obj-instance.h"
#include "ns/include/platform/alarm.h"
#include "ns/sys/core/common/timer.h"

// ---- Timer objects
// timer = nespy.Timer(inst=instance, cb=callback) # create timer object with it's callback and instance
// timer.start(100)                                # create timer with 100ms interval

const mp_obj_type_t py_timer_type;

typedef struct _py_timer_obj py_timer_obj_t;
typedef struct _py_timer_list py_timer_list_obj_t;

struct _py_timer_obj {
    mp_obj_base_t base;
    mp_obj_t callback;
    instance_t *instance;
    timer_t timer;
    uint32_t interval;
    py_timer_obj_t *next;
};

struct _py_timer_list {
    py_timer_obj_t *head;
};

static py_timer_list_obj_t py_timer_list;

static void py_timer_list_add(py_timer_obj_t *timer_obj);
static void py_timer_list_remove(py_timer_obj_t *timer_obj);

void timer_handler(timer_t *timer)
{
    py_timer_obj_t *head = py_timer_list.head;
    py_timer_obj_t *cur;
    for (cur = head; cur; cur = cur->next) {
        if ((timer_t *)&cur->timer == timer) {
            py_timer_list_remove(cur);
            mp_call_function_0(cur->callback);
            break;
        }
    }
}

STATIC mp_obj_t py_timer_make_new(const mp_obj_type_t *type,
                                  size_t n_args,
                                  size_t n_kw,
                                  const mp_obj_t *all_args)
{
    if (n_args == 0 && n_kw == 0) {
        nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError,
                  "ns: invalid argument!"));
    }
    enum { ARG_inst, ARG_cb };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_inst, MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_cb, MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_obj = mp_const_none} },
    };
    // parse args
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all_kw_array(n_args, n_kw, all_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);
    // make sure we got instance and callback argument
    if (args[ARG_inst].u_obj == mp_const_none && args[ARG_cb].u_obj == mp_const_none) {
        nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError,
                  "ns: please specify instance and callback argument"));
    }
    // create timer obj
    py_timer_obj_t *tim = m_new_obj(py_timer_obj_t);
    py_instance_obj_t *inst = MP_OBJ_TO_PTR(args[ARG_inst].u_obj);
    tim->base.type = &py_timer_type;
    tim->callback = args[ARG_cb].u_obj;
    tim->instance = (instance_t *)inst->instance;
    tim->interval = 0;
    tim->timer.handler = timer_handler;
    return MP_OBJ_FROM_PTR(tim);
}

STATIC mp_obj_t py_timer_start(mp_obj_t self_in,
                               mp_obj_t interval_in)
{
    py_timer_obj_t *self = MP_OBJ_TO_PTR(self_in);
    self->interval = (uint32_t)mp_obj_get_int(interval_in);
    uint32_t now = ns_plat_alarm_get_now();
    // add this timer to the list & start
    py_timer_list_add(self);
    timer_start(self->instance, (timer_t *)&self->timer, now, self->interval);
    return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_2(py_timer_start_obj, py_timer_start);

STATIC const mp_rom_map_elem_t py_timer_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_start), MP_ROM_PTR(&py_timer_start_obj) },
};

STATIC MP_DEFINE_CONST_DICT(py_timer_locals_dict, py_timer_locals_dict_table);

const mp_obj_type_t py_timer_type = {
    { &mp_type_type },
    .name = MP_QSTR_Timer,
    .make_new = py_timer_make_new,
    .locals_dict = (mp_obj_dict_t *)&py_timer_locals_dict,
};

static void py_timer_list_add(py_timer_obj_t *timer_obj)
{
    py_timer_list_remove(timer_obj);
    py_timer_obj_t *head = py_timer_list.head;
    if (head == NULL) {
        // update timer list head
        py_timer_list.head = timer_obj;
        timer_obj->next = NULL;
    } else {
        py_timer_obj_t *cur;
        for (cur = head; cur; cur = cur->next) {
            if (cur->next == NULL) {
                cur->next = timer_obj;
                timer_obj->next = NULL;
                break;
            }
        }
    }
}

static void py_timer_list_remove(py_timer_obj_t *timer_obj)
{
    if (timer_obj->next == timer_obj) {
        goto exit;
    }
    py_timer_obj_t *head = py_timer_list.head;
    if (head == timer_obj) {
        // update timer list head
        py_timer_list.head = timer_obj->next;
    } else {
        for (py_timer_obj_t *cur = head; cur; cur = cur->next) {
            if (cur->next == timer_obj) {
                cur->next = timer_obj->next;
                break;
            }
        }
    }
    timer_obj->next = timer_obj;
exit:
    return;
}