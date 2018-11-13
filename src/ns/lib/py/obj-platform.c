#include "py/nlr.h"
#include "py/runtime.h"

// Example usage to Platform objects
//
//      platform = nespy.Platform()
//      platform.process_update() # use to update low level driver process

const mp_obj_type_t ns_plat_type;

typedef struct _ns_plat_obj_t {
    mp_obj_base_t base;
} ns_plat_obj_t;

#if defined(UNIX)
extern void unix_process_update(void);
#endif

STATIC mp_obj_t ns_plat_make_new(const mp_obj_type_t *type,
                                 size_t n_args,
                                 size_t n_kw,
                                 const mp_obj_t *all_args)
{
    // check arguments
    mp_arg_check_num(n_args, n_kw, 0, 0, true);

    // create platform obj
    ns_plat_obj_t *plat = m_new_obj(ns_plat_obj_t);
    plat->base.type = &ns_plat_type;

    return MP_OBJ_FROM_PTR(plat);
}

STATIC mp_obj_t ns_plat_process_update(mp_obj_t self_in)
{
#if defined(UNIX)
    unix_process_update();
#endif
    return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_1(ns_plat_process_update_obj, ns_plat_process_update);

STATIC const mp_rom_map_elem_t ns_plat_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_process_update), MP_ROM_PTR(&ns_plat_process_update_obj) },
};

STATIC MP_DEFINE_CONST_DICT(ns_plat_locals_dict, ns_plat_locals_dict_table);

const mp_obj_type_t ns_plat_type = {
    { &mp_type_type },
    .name = MP_QSTR_Platform,
    .make_new = ns_plat_make_new,
    .locals_dict = (mp_obj_dict_t *)&ns_plat_locals_dict,
};
