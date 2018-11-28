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
typedef struct _py_tasklet_list py_tasklet_list_obj_t;

struct _py_tasklet_obj {
    mp_obj_base_t base;
    mp_obj_t callback;
    ns_instance_t *instance;
    tasklet_t tasklet;
    py_tasklet_obj_t *next;
};

struct _py_tasklet_list {
    py_tasklet_obj_t *head;
};

static py_tasklet_list_obj_t py_tasklet_list;

static void
py_tasklet_list_add(py_tasklet_obj_t *tasklet_obj);

static void
py_tasklet_list_remove(py_tasklet_obj_t *tasklet_obj);

static void
tasklet_handler(tasklet_t *tasklet)
{
    py_tasklet_obj_t *head = py_tasklet_list.head;
    py_tasklet_obj_t *cur;
    for (cur = head; cur; cur = cur->next) {
        if ((tasklet_t *)&cur->tasklet == tasklet) {
            py_tasklet_list_remove(cur);
            mp_call_function_0(cur->callback);
            break;
        }
    }
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
    tasklet->tasklet.handler = tasklet_handler;
    return MP_OBJ_FROM_PTR(tasklet);
}

STATIC mp_obj_t
py_tasklet_post(mp_obj_t self_in)
{
    py_tasklet_obj_t *self = MP_OBJ_TO_PTR(self_in);
    // add this task to the list & post
    py_tasklet_list_add(self);
    tasklet_post((void *)self->instance, (tasklet_t *)&self->tasklet);
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

static void
py_tasklet_list_add(py_tasklet_obj_t *tasklet_obj)
{
    py_tasklet_list_remove(tasklet_obj);
    py_tasklet_obj_t *head = py_tasklet_list.head;
    if (head == NULL) {
        // update tasklet list head
        py_tasklet_list.head = tasklet_obj;
        tasklet_obj->next = NULL;
    } else {
        py_tasklet_obj_t *cur;
        for (cur = head; cur; cur = cur->next) {
            if (cur->next == NULL) {
                cur->next = tasklet_obj;
                tasklet_obj->next = NULL;
                break;
            }
        }
    }
}

static void
py_tasklet_list_remove(py_tasklet_obj_t *tasklet_obj)
{
    VERIFY_OR_EXIT(tasklet_obj->next != tasklet_obj);
    py_tasklet_obj_t *head = py_tasklet_list.head;
    if (head == tasklet_obj) {
        // update tasklet list head
        py_tasklet_list.head = tasklet_obj->next;
    } else {
        for (py_tasklet_obj_t *cur = head; cur; cur = cur->next) {
            if (cur->next == tasklet_obj) {
                cur->next = tasklet_obj->next;
                break;
            }
        }
    }
    tasklet_obj->next = tasklet_obj;
exit:
    return;
}
