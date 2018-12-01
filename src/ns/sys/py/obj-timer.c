#include "py/nlr.h"
#include "py/runtime.h"
#include "ns/sys/py/obj-instance.h"
#include "ns/include/platform/alarm-milli.h"
#include "ns/include/platform/alarm-micro.h"
#include "ns/sys/core/common/timer.h"

// ---- Timer objects
// # create timer object with it's callback and instance
// timer_ms = nespy.TimerMilli(inst=instance, cb=timer_ms_cb)
// timer_us = nespy.TimerMicro(inst=instance, cb=timer_us_cb)
// ----
// # create timer with 100ms and 1000us interval
// timer_ms.start(100)
// timer_us.start(1000)

const mp_obj_type_t py_timer_milli_type;
#if NS_CONFIG_ENABLE_PLATFORM_USEC_TIMER
const mp_obj_type_t py_timer_micro_type;
#endif // NS_CONFIG_ENABLE_PLATFORM_USEC_TIMER

typedef struct _py_timer_obj py_timer_obj_t;
struct _py_timer_obj {
    mp_obj_base_t base;
    mp_obj_t callback;
    ns_instance_t *instance;
    timer_t timer;
    uint32_t interval;
};

void
timer_handler_func(timer_t *timer)
{
    mp_obj_t callback = *(mp_obj_t *)timer->handler.arg;
    mp_call_function_0(callback);
}

STATIC py_timer_obj_t *
py_timer_make_new(const mp_obj_type_t *type,
                  size_t n_args,
                  size_t n_kw,
                  const mp_obj_t *all_args)
{
    if (n_args == 0 && n_kw != 2) {
        nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError,
                  "ns: invalid number of argument!"));
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
    if (args[ARG_inst].u_obj == mp_const_none || args[ARG_cb].u_obj == mp_const_none) {
        nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError,
                  "ns: please specify instance and callback argument"));
    }
    // create timer obj
    py_timer_obj_t *tim = m_new_obj(py_timer_obj_t);
    py_instance_obj_t *inst = MP_OBJ_TO_PTR(args[ARG_inst].u_obj);
    tim->callback = args[ARG_cb].u_obj;
    tim->instance = inst->instance;
    tim->interval = 0;
    return tim;
}

STATIC mp_obj_t
py_timer_milli_make_new(const mp_obj_type_t *type,
                        size_t n_args,
                        size_t n_kw,
                        const mp_obj_t *all_args)
{
    py_timer_obj_t *self = py_timer_make_new(type, n_args, n_kw, all_args);
    self->base.type = &py_timer_milli_type;
    timer_milli_ctor((void *)self->instance, &self->timer, &timer_handler_func, (void *)&self->callback);
    return MP_OBJ_FROM_PTR(self);
}

STATIC mp_obj_t
py_timer_milli_start(mp_obj_t self_in,
                     mp_obj_t interval_in)
{
    py_timer_obj_t *self = MP_OBJ_TO_PTR(self_in);
    self->interval = (uint32_t)mp_obj_get_int(interval_in);
    timer_milli_start(&self->timer, self->interval);
    return mp_const_none;
}

#if NS_CONFIG_ENABLE_PLATFORM_USEC_TIMER
STATIC mp_obj_t
py_timer_micro_make_new(const mp_obj_type_t *type,
                        size_t n_args,
                        size_t n_kw,
                        const mp_obj_t *all_args)
{
    py_timer_obj_t *self = py_timer_make_new(type, n_args, n_kw, all_args);
    self->base.type = &py_timer_micro_type;
    timer_micro_ctor((void *)self->instance, &self->timer, &timer_handler_func, (void *)&self->callback);
    return MP_OBJ_FROM_PTR(self);
}

STATIC mp_obj_t
py_timer_micro_start(mp_obj_t self_in,
                     mp_obj_t interval_in)
{
    py_timer_obj_t *self = MP_OBJ_TO_PTR(self_in);
    self->interval = (uint32_t)mp_obj_get_int(interval_in);
    timer_micro_start(&self->timer, self->interval);
    return mp_const_none;
}
#endif // NS_CONFIG_ENABLE_PLATFORM_USEC_TIMER

STATIC MP_DEFINE_CONST_FUN_OBJ_2(py_timer_milli_start_obj, py_timer_milli_start);
#if NS_CONFIG_ENABLE_PLATFORM_USEC_TIMER
STATIC MP_DEFINE_CONST_FUN_OBJ_2(py_timer_micro_start_obj, py_timer_micro_start);
#endif // NS_CONFIG_ENABLE_PLATFORM_USEC_TIMER

STATIC const mp_rom_map_elem_t py_timer_milli_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_start), MP_ROM_PTR(&py_timer_milli_start_obj) },
};
#if NS_CONFIG_ENABLE_PLATFORM_USEC_TIMER
STATIC const mp_rom_map_elem_t py_timer_micro_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_start), MP_ROM_PTR(&py_timer_micro_start_obj) },
};
#endif // NS_CONFIG_ENABLE_PLATFORM_USEC_TIMER

STATIC MP_DEFINE_CONST_DICT(py_timer_milli_locals_dict, py_timer_milli_locals_dict_table);
#if NS_CONFIG_ENABLE_PLATFORM_USEC_TIMER
STATIC MP_DEFINE_CONST_DICT(py_timer_micro_locals_dict, py_timer_micro_locals_dict_table);
#endif // NS_CONFIG_ENABLE_PLATFORM_USEC_TIMER

const mp_obj_type_t py_timer_milli_type = {
    { &mp_type_type },
    .name = MP_QSTR_TimerMilli,
    .make_new = py_timer_milli_make_new,
    .locals_dict = (mp_obj_dict_t *)&py_timer_milli_locals_dict,
};

#if NS_CONFIG_ENABLE_PLATFORM_USEC_TIMER
const mp_obj_type_t py_timer_micro_type = {
    { &mp_type_type },
    .name = MP_QSTR_TimerMicro,
    .make_new = py_timer_micro_make_new,
    .locals_dict = (mp_obj_dict_t *)&py_timer_micro_locals_dict,
};
#endif // NS_CONFIG_ENABLE_PLATFORM_USEC_TIMER
