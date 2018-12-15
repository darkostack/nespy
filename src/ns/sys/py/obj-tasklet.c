#include "py/nlr.h"
#include "py/runtime.h"
#include "ns/sys/py/obj-instance.h"
#include "ns/include/tasklet.h"
#include "ns/sys/core/common/tasklet.h"

// ---- Tasklet objects
// # create tasklet object with it's task function callback
// task = nespy.Tasklet(inst=instance, cb=task_cb)
// ----
// # post the task to the tasklet queue
// task.post()

const mp_obj_type_t py_tasklet_type;

typedef struct _py_tasklet_obj py_tasklet_obj_t;
struct _py_tasklet_obj {
    mp_obj_base_t base;
    mp_obj_t callback;
    ns_instance_t *instance;
    tasklet_t tasklet;
};

static void
tasklet_handler_func(void *tasklet)
{
    mp_obj_t callback = *((mp_obj_t *)((tasklet_t *)tasklet)->handler.arg);
    mp_call_function_0(callback);
}

STATIC mp_obj_t
py_tasklet_make_new(const mp_obj_type_t *type,
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
    // make sure we got instance and task argument
    if (args[ARG_inst].u_obj == mp_const_none || args[ARG_cb].u_obj == mp_const_none) {
        nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError,
                  "ns: please specify instance and task argument"));
    }
    // create tasklet obj
    py_tasklet_obj_t *tasklet = m_new_obj(py_tasklet_obj_t);
    py_instance_obj_t *inst = MP_OBJ_TO_PTR(args[ARG_inst].u_obj);
    tasklet->base.type = &py_tasklet_type;
    tasklet->callback = args[ARG_cb].u_obj;
    tasklet->instance = inst->instance;

    // tasklet contructor
    tasklet_ctor(tasklet->instance,
                 &tasklet->tasklet,
                 &tasklet_handler_func,
                 &tasklet->callback);

    return MP_OBJ_FROM_PTR(tasklet);
}

STATIC mp_obj_t
py_tasklet_post(mp_obj_t self_in)
{
    py_tasklet_obj_t *self = MP_OBJ_TO_PTR(self_in);
    tasklet_post(&self->tasklet);
    return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_tasklet_post_obj, py_tasklet_post);

STATIC const mp_rom_map_elem_t py_tasklet_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_post), MP_ROM_PTR(&py_tasklet_post_obj) },
};

STATIC MP_DEFINE_CONST_DICT(py_tasklet_locals_dict, py_tasklet_locals_dict_table);

const mp_obj_type_t py_tasklet_type = {
    { &mp_type_type },
    .name = MP_QSTR_Tasklet,
    .make_new = py_tasklet_make_new,
    .locals_dict = (mp_obj_dict_t *)&py_tasklet_locals_dict,
};
