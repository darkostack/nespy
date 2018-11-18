#include "py/nlr.h"
#include "py/runtime.h"

extern const mp_obj_type_t py_instance_type;
extern const mp_obj_type_t py_platform_type;
extern const mp_obj_type_t py_timer_type;
extern const mp_obj_type_t py_tasklet_type;

STATIC const mp_rom_map_elem_t nespy_module_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_OBJ_NEW_QSTR(MP_QSTR_nespy) },
    { MP_ROM_QSTR(MP_QSTR_Instance), MP_ROM_PTR(&py_instance_type) },
    { MP_ROM_QSTR(MP_QSTR_Platform), MP_ROM_PTR(&py_platform_type) },
    { MP_ROM_QSTR(MP_QSTR_Timer), MP_ROM_PTR(&py_timer_type) },
    { MP_ROM_QSTR(MP_QSTR_Tasklet), MP_ROM_PTR(&py_tasklet_type) },
};

STATIC MP_DEFINE_CONST_DICT(nespy_module_globals, nespy_module_globals_table);

// Nespy module imported to Micropython (see mpconfigport.h)
const mp_obj_module_t mp_module_nespy = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t *)&nespy_module_globals,
};
