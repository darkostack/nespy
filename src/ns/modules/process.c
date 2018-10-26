#include "py/nlr.h"
#include "py/runtime.h"
#include "ns/contiki.h"
#include "ns/modules/process.h"

// Example usage to Process objects
//
//      process = ns.Process()
//      process.run() # run the internal network stack process

typedef struct _ns_process_obj_t {
    mp_obj_base_t base;
} ns_process_obj_t;

// process = ns.Process() constructor
STATIC mp_obj_t ns_process_make_new(const mp_obj_type_t *type,
                                    size_t n_args,
                                    size_t n_kw,
                                    const mp_obj_t *args)
{
    // check arguments (min: 0, max: 0)
    mp_arg_check_num(n_args, n_kw, 0, 0, true);

    // create process object
    ns_process_obj_t *process = m_new_obj(ns_process_obj_t);
    process->base.type = &ns_process_type;

    return MP_OBJ_FROM_PTR(process);
}

// process.run()
STATIC mp_obj_t ns_process_run(void)
{
    return mp_obj_new_int(process_run());
}

// print(process)
STATIC void ns_process_print(const mp_print_t *print,
                             mp_obj_t self_in,
                             mp_print_kind_t kind)
{
    mp_printf(print, "ns: process events waiting (%d)", process_nevents());
}

STATIC MP_DEFINE_CONST_FUN_OBJ_0(ns_process_run_obj, ns_process_run);

STATIC const mp_rom_map_elem_t ns_process_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_run), MP_ROM_PTR(&ns_process_run_obj) },
};

STATIC MP_DEFINE_CONST_DICT(ns_process_locals_dict, ns_process_locals_dict_table);

const mp_obj_type_t ns_process_type = {
    { &mp_type_type },
    .name = MP_QSTR_Process,
    .print = ns_process_print,
    .make_new = ns_process_make_new,
    .locals_dict = (mp_obj_dict_t *)&ns_process_locals_dict,
};
