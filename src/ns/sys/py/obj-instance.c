#include "py/nlr.h"
#include "py/runtime.h"
#include "ns/sys/py/obj-instance.h"

// ---- Instance objects
// # create and initialize instance object
// instance = nespy.Instance()
// ----
// # check init status
// instance.is_initialized()

const mp_obj_type_t py_instance_type;
static bool s_instance_obj_is_created = false;

STATIC mp_obj_t
py_instance_make_new(const mp_obj_type_t *type,
                     size_t n_args,
                     size_t n_kw,
                     const mp_obj_t *all_args)
{
    mp_arg_check_num(n_args, n_kw, 0, 0, true);
    if (s_instance_obj_is_created) {
        nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError,
                  "ns: can't create more than one instance object"));

    }
    // create instance obj
    py_instance_obj_t *instance = m_new_obj(py_instance_obj_t);
    instance->base.type = &py_instance_type;
    instance->instance = ns_instance_init();
    s_instance_obj_is_created = true;
    return MP_OBJ_FROM_PTR(instance);
}

STATIC mp_obj_t
py_instance_is_initialized(mp_obj_t self_in)
{
    py_instance_obj_t *self = MP_OBJ_TO_PTR(self_in);
    bool ret = ns_instance_is_initialized(self->instance);
    return mp_obj_new_bool(ret);
}

STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_instance_is_initialized_obj, py_instance_is_initialized);

STATIC const mp_rom_map_elem_t py_instance_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_is_initialized), MP_ROM_PTR(&py_instance_is_initialized_obj) },
};

STATIC MP_DEFINE_CONST_DICT(py_instance_locals_dict, py_instance_locals_dict_table);

const mp_obj_type_t py_instance_type = {
    { &mp_type_type },
    .name = MP_QSTR_Instance,
    .make_new = py_instance_make_new,
    .locals_dict = (mp_obj_dict_t *)&py_instance_locals_dict,
};
