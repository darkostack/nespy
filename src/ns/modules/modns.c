#include "py/nlr.h"
#include "py/runtime.h"
#include <stdio.h>

extern const mp_obj_type_t ns_hello_type;
// TODO: this protection is not necessary when unix is fully port to netstack
#if !defined(UNIX)
extern const mp_obj_type_t ns_init_type;
extern const mp_obj_type_t ns_process_type;
extern const mp_obj_type_t ns_thread_type;
#endif

STATIC const mp_rom_map_elem_t ns_module_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_OBJ_NEW_QSTR(MP_QSTR_ns) },
    { MP_ROM_QSTR(MP_QSTR_Hello), MP_ROM_PTR(&ns_hello_type) },
#if !defined(UNIX)
    { MP_ROM_QSTR(MP_QSTR_Init), MP_ROM_PTR(&ns_init_type) },
    { MP_ROM_QSTR(MP_QSTR_Process), MP_ROM_PTR(&ns_process_type) },
    { MP_ROM_QSTR(MP_QSTR_Thread), MP_ROM_PTR(&ns_thread_type) },
#endif
};

STATIC MP_DEFINE_CONST_DICT(ns_module_globals, ns_module_globals_table);

const mp_obj_module_t ns_module = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t *)&ns_module_globals,
};
