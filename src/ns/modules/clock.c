#include "py/nlr.h"
#include "py/runtime.h"
#include "ns/contiki.h"

const mp_obj_type_t ns_clock_type;

typedef struct _ns_clock_obj_t {
    mp_obj_base_t base;
} ns_clock_obj_t;

STATIC mp_obj_t ns_clock_make_new(const mp_obj_type_t *type,
                                  size_t n_args,
                                  size_t n_kw,
                                  const mp_obj_t *all_args)
{
    // check arguments
    mp_arg_check_num(n_args, n_kw, 0, 0, true);

    // create clock object
    ns_clock_obj_t *clock = m_new_obj(ns_clock_obj_t);
    clock->base.type = &ns_clock_type;

    return MP_OBJ_FROM_PTR(clock);
}

STATIC mp_obj_t ns_clock_get(mp_obj_t self_in)
{
    return mp_obj_new_int(clock_time());
}

STATIC MP_DEFINE_CONST_FUN_OBJ_1(ns_clock_get_obj, ns_clock_get);

STATIC const mp_rom_map_elem_t ns_clock_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_get), MP_ROM_PTR(&ns_clock_get_obj) },
};

STATIC MP_DEFINE_CONST_DICT(ns_clock_locals_dict, ns_clock_locals_dict_table);

const mp_obj_type_t ns_clock_type = {
    { &mp_type_type },
    .name = MP_QSTR_Clock,
    .make_new = ns_clock_make_new,
    .locals_dict = (mp_obj_dict_t *)&ns_clock_locals_dict,
};
