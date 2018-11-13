#include "py/nlr.h"
#include "py/runtime.h"
#include <stdio.h>

extern const mp_obj_type_t ns_hello_type;
extern const mp_obj_type_t ns_init_type;
extern const mp_obj_type_t ns_process_type;
extern const mp_obj_type_t ns_thread_type;
extern const mp_obj_type_t ns_clock_type;
extern const mp_obj_type_t ns_plat_type;
#if APP_CONF_WITH_COAP
extern const mp_obj_type_t ns_coap_resource_type;
#endif
extern const mp_obj_type_t ns_etimer_type;

STATIC const mp_rom_map_elem_t ns_module_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_OBJ_NEW_QSTR(MP_QSTR_ns) },
    { MP_ROM_QSTR(MP_QSTR_Hello), MP_ROM_PTR(&ns_hello_type) },
    { MP_ROM_QSTR(MP_QSTR_Init), MP_ROM_PTR(&ns_init_type) },
    { MP_ROM_QSTR(MP_QSTR_Process), MP_ROM_PTR(&ns_process_type) },
    { MP_ROM_QSTR(MP_QSTR_Thread), MP_ROM_PTR(&ns_thread_type) },
    { MP_ROM_QSTR(MP_QSTR_Clock), MP_ROM_PTR(&ns_clock_type) },
    { MP_ROM_QSTR(MP_QSTR_Platform), MP_ROM_PTR(&ns_plat_type) },
#if APP_CONF_WITH_COAP
    { MP_ROM_QSTR(MP_QSTR_CoapResource), MP_ROM_PTR(&ns_coap_resource_type) },
#endif
    { MP_ROM_QSTR(MP_QSTR_Etimer), MP_ROM_PTR(&ns_etimer_type) },
};

STATIC MP_DEFINE_CONST_DICT(ns_module_globals, ns_module_globals_table);

const mp_obj_module_t ns_module = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t *)&ns_module_globals,
};
