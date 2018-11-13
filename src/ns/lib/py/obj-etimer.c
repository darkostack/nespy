#include "py/nlr.h"
#include "py/runtime.h"
#include "ns/contiki.h"
#include "ns/contiki-net.h"
#include "ns/sys/int-master.h"
#include "ns/lib/py/obj-etimer.h"
#include <stdio.h>

// Example usage to Etimer (Event Timer) objects
//
//      # set timer object with 1000 ms period
//      timer = nespy.Etimer(period=1000)
//
//      timer.start()    # start the timer
//      timer.expired()  # get timer is expired status
//      timer.reset()    # reset the timer with it's previous period setting
//      timer.stop()     # stop the timer
//      timer.callback() # call defined timer callback

const mp_obj_type_t ns_etimer_type;

STATIC mp_obj_t ns_etimer_make_new(const mp_obj_type_t *type,
                                   size_t n_args,
                                   size_t n_kw,
                                   const mp_obj_t *all_args)
{
    if (n_args == 0 && n_kw == 0) {
        nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError,
                  "ns: invalid arguments"));
    }

    enum {ARG_period};
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_period,   MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 0} },
    };

    // parse args
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all_kw_array(n_args, n_kw, all_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    // check the argument
    if (args[ARG_period].u_int == 0) {
        nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError,
                  "ns: timer period can't be 0"));
    }

    // create etimer object
    ns_etimer_obj_t *timer = m_new_obj(ns_etimer_obj_t);
    timer->base.type = &ns_etimer_type;
    timer->period = (clock_time_t)args[ARG_period].u_int;

    return MP_OBJ_FROM_PTR(timer);
}

STATIC mp_obj_t ns_etimer_start(mp_obj_t self_in)
{
    ns_etimer_obj_t *self = MP_OBJ_FROM_PTR(self_in);
    uint32_t div = (self->period < 1000) ? (1000 / self->period) : (self->period / 1000);
    clock_time_t period = CLOCK_SECOND / div;
    etimer_set(&self->timer, period);
    return mp_const_none;
}

STATIC mp_obj_t ns_etimer_expired(mp_obj_t self_in)
{
    ns_etimer_obj_t *self = MP_OBJ_FROM_PTR(self_in);
    return etimer_expired(&self->timer) ? mp_obj_new_bool(true) : mp_obj_new_bool(false);
}

STATIC mp_obj_t ns_etimer_reset(mp_obj_t self_in)
{
    ns_etimer_obj_t *self = MP_OBJ_FROM_PTR(self_in);
    etimer_reset(&self->timer);
    return mp_const_none;
}

STATIC mp_obj_t ns_etimer_stop(mp_obj_t self_in)
{
    ns_etimer_obj_t *self = MP_OBJ_FROM_PTR(self_in);
    etimer_stop(&self->timer);
    return mp_const_none;
}

STATIC mp_obj_t ns_etimer_event(mp_obj_t self_in)
{
    return mp_obj_new_int_from_uint(PROCESS_EVENT_TIMER);
}

STATIC MP_DEFINE_CONST_FUN_OBJ_1(ns_etimer_start_obj, ns_etimer_start);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(ns_etimer_expired_obj, ns_etimer_expired);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(ns_etimer_reset_obj, ns_etimer_reset);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(ns_etimer_stop_obj, ns_etimer_stop);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(ns_etimer_event_obj, ns_etimer_event);

STATIC const mp_rom_map_elem_t ns_etimer_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_start), MP_ROM_PTR(&ns_etimer_start_obj) },
    { MP_ROM_QSTR(MP_QSTR_expired), MP_ROM_PTR(&ns_etimer_expired_obj) },
    { MP_ROM_QSTR(MP_QSTR_reset), MP_ROM_PTR(&ns_etimer_reset_obj) },
    { MP_ROM_QSTR(MP_QSTR_stop), MP_ROM_PTR(&ns_etimer_stop_obj) },
    { MP_ROM_QSTR(MP_QSTR_event), MP_ROM_PTR(&ns_etimer_event_obj) },
};

STATIC MP_DEFINE_CONST_DICT(ns_etimer_locals_dict, ns_etimer_locals_dict_table);

const mp_obj_type_t ns_etimer_type = {
    { &mp_type_type },
    .name = MP_QSTR_Etimer,
    .make_new = ns_etimer_make_new,
    .locals_dict = (mp_obj_dict_t *)&ns_etimer_locals_dict,
};
