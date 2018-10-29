#include "py/nlr.h"
#include "py/runtime.h"

// TODO: once unix ports is fully ported to ns, remove this checking
#if !defined(UNIX)
#include "ns/contiki.h"
#include "ns/modules/process.h"
#endif

#include <stdio.h>

// Example usage to Hello objects
//
//      ns.Hello("World") # output: Hello World!

#if !defined(UNIX)
PROCESS(modules_autostart, "modules autostart process");
AUTOSTART_PROCESSES(&modules_autostart);

PROCESS_THREAD(modules_autostart, ev, data)
{
    PROCESS_BEGIN();
    while (1) {
        PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_POLL);
        printf("ns: modules autostart: event poll!\n");
    }

    PROCESS_END();
}
#endif

STATIC mp_obj_t ns_hello(mp_obj_t what)
{
    printf("Hello %s!\n", mp_obj_str_get_str(what));
    return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_1(ns_hello_obj, ns_hello);

STATIC const mp_rom_map_elem_t ns_module_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_OBJ_NEW_QSTR(MP_QSTR_ns) },
    { MP_ROM_QSTR(MP_QSTR_Hello), (mp_obj_t)&ns_hello_obj },
#if !defined(UNIX)
    { MP_ROM_QSTR(MP_QSTR_Process), MP_ROM_PTR(&ns_process_type) },
    { MP_ROM_QSTR(MP_QSTR_Thread), MP_ROM_PTR(&ns_thread_type) },
#endif
};

STATIC MP_DEFINE_CONST_DICT(ns_module_globals, ns_module_globals_table);

const mp_obj_module_t ns_module = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t *)&ns_module_globals,
};
