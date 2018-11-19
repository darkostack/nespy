#include "py/nlr.h"
#include "py/runtime.h"
#include "ns/sys/py/obj-instance.h"
#include "ns/include/platform/system.h"
#include "ns/include/tasklet.h"

// ---- Platform objects
// # create platform object with it's ID
//  platform = nespy.Platform(inst=instance, id=1)
// ----
// # initialize platform system
// platform.system_init()
// ----
// # process platform drivers task
// platform.process_drivers()
// ----
// # run platform tasklet engine
// platform.tasklet_run()

const mp_obj_type_t py_platform_type;
static bool platform_obj_is_created = false;

typedef struct _py_platform_obj_t {
    mp_obj_base_t base;
    ns_instance_t instance;
    uint32_t node_id;
} py_platform_obj_t;

STATIC mp_obj_t py_platform_make_new(const mp_obj_type_t *type,
                                     size_t n_args,
                                     size_t n_kw,
                                     const mp_obj_t *all_args)
{
    if (n_args == 0 && n_kw != 2) {
        nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError,
                  "ns: invalid number of argument!"));
    }
    if (platform_obj_is_created) {
        nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError,
                  "ns: can't create more than one instance object"));
    }
    enum { ARG_inst, ARG_id };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_inst, MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_id, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 0} },
    };
    // parse args
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all_kw_array(n_args, n_kw, all_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);
    // check arguments
    if (args[ARG_inst].u_obj == mp_const_none || args[ARG_id].u_int == 0) {
        nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError,
                  "ns: please specify instance and id argument"));
    }
    // create platform object
    py_platform_obj_t *plat = m_new_obj(py_platform_obj_t);
    py_instance_obj_t *inst = MP_OBJ_TO_PTR(args[ARG_inst].u_obj);
    plat->base.type = &py_platform_type;
    plat->instance = inst->instance;
    plat->node_id = (uint32_t)args[ARG_id].u_int;
    return MP_OBJ_FROM_PTR(plat);
}

STATIC mp_obj_t py_platform_system_init(mp_obj_t self_in)
{
    py_platform_obj_t *self = MP_OBJ_TO_PTR(self_in);
    ns_plat_sys_init(self->node_id);
    return mp_const_none;
}

STATIC mp_obj_t py_platform_process_drivers(mp_obj_t self_in)
{
    py_platform_obj_t *self = MP_OBJ_TO_PTR(self_in);
    ns_plat_sys_process_drivers(self->instance);
    return mp_const_none;
}

STATIC mp_obj_t py_platform_tasklet_run(mp_obj_t self_in)
{
    py_platform_obj_t *self = MP_OBJ_TO_PTR(self_in);
    ns_tasklet_process(self->instance);
    return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_platform_system_init_obj, py_platform_system_init);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_platform_process_drivers_obj, py_platform_process_drivers);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_platform_tasklet_run_obj, py_platform_tasklet_run);

STATIC const mp_rom_map_elem_t py_platform_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_system_init), MP_ROM_PTR(&py_platform_system_init_obj) },
    { MP_ROM_QSTR(MP_QSTR_process_drivers), MP_ROM_PTR(&py_platform_process_drivers_obj) },
    { MP_ROM_QSTR(MP_QSTR_tasklet_run), MP_ROM_PTR(&py_platform_tasklet_run_obj) },
};

STATIC MP_DEFINE_CONST_DICT(py_platform_locals_dict, py_platform_locals_dict_table);

const mp_obj_type_t py_platform_type = {
    { &mp_type_type },
    .name = MP_QSTR_Platform,
    .make_new = py_platform_make_new,
    .locals_dict = (mp_obj_dict_t *)&py_platform_locals_dict,
};
