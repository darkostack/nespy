#if defined(UNIX)
#include "py/nlr.h"
#include "py/runtime.h"

const mp_obj_type_t ns_unix_type;

typedef struct _ns_unix_obj_t {
    mp_obj_base_t base;
} ns_unix_obj_t;

extern void rtimer_alarm_process(void);
extern void etimer_pending_process(void);

STATIC mp_obj_t ns_unix_make_new(const mp_obj_type_t *type,
                                 size_t n_args,
                                 size_t n_kw,
                                 const mp_obj_t *all_args)
{
    // check arguments
    mp_arg_check_num(n_args, n_kw, 0, 0, true);

    // create unix obj
    ns_unix_obj_t *unix = m_new_obj(ns_unix_obj_t);
    unix->base.type = &ns_unix_type;

    return MP_OBJ_FROM_PTR(unix);
}

STATIC mp_obj_t ns_unix_process_update(mp_obj_t self_in)
{
    rtimer_alarm_process();
    etimer_pending_process();
    return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_1(ns_unix_process_update_obj, ns_unix_process_update);

STATIC const mp_rom_map_elem_t ns_unix_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_process_update), MP_ROM_PTR(&ns_unix_process_update_obj) },
};

STATIC MP_DEFINE_CONST_DICT(ns_unix_locals_dict, ns_unix_locals_dict_table);

const mp_obj_type_t ns_unix_type = {
    { &mp_type_type },
    .name = MP_QSTR_Unix,
    .make_new = ns_unix_make_new,
    .locals_dict = (mp_obj_dict_t *)&ns_unix_locals_dict,
};
#endif // #if defined(UNIX)
