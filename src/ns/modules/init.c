#include "py/nlr.h"
#include "py/runtime.h"
#include "ns/contiki.h"
#include "ns/contiki-net.h"
#include "ns/sys/node-id.h"
#include "ns/sys/platform.h"
#include "ns/sys/energest.h"
#include "ns/sys/stack-check.h"
#include "ns/dev/watchdog.h"
#include "ns/net/app-layer/coap/coap-engine.h"
#include "ns/services/rpl-border-router/rpl-border-router.h"
#include "ns/services/orchestra/orchestra.h"
#include "ns/services/shell/serial-shell.h"
#include "ns/services/simple-energest/simple-energest.h"
#include "ns/services/tsch-cs/tsch-cs.h"

#include <stdint.h>
#include <stdio.h>

// Example usage to Init objects
//
//      init = ns.Init()
//
//      init.platform(1)            # init platform stage one
//      init.clock()
//      init.rtimer()
//      init.process()
//      init.etimer()
//      init.ctimer()
//      init.watchdog()
//      init.energest()
//      init.stack_check()
//      init.platform(2)            # init platform stage two
//      init.netstack()
//      init.node_id()
//      init.ipv6_addr()
//      init.platform(3)            # init platform stage three
//      init.rpl_border_router()
//      init.orchestra()
//      init.shell()
//      init.coap_engine()
//      init.simple_energest()
//      init.tsch_cs_adaptions()

const mp_obj_type_t ns_init_type;

typedef struct _ns_init_obj_t {
    mp_obj_base_t base;
} ns_init_obj_t;

static bool is_init_obj_created = false;

// init = ns.Init() # init object constructor
STATIC mp_obj_t ns_init_make_new(const mp_obj_type_t *type,
                                 size_t n_args,
                                 size_t n_kw,
                                 const mp_obj_t *all_args)
{
    // check arguments (min: 0, max: 0)
    mp_arg_check_num(n_args, n_kw, 0, 0, true);

    if (is_init_obj_created) {
        nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError,
                  "ns: can't create more than one init object"));
    }

    // create init object
    ns_init_obj_t *init = m_new_obj(ns_init_obj_t);
    memset(init, 0, sizeof(*init));
    init->base.type = &ns_init_type;

    is_init_obj_created = true;

    return MP_OBJ_FROM_PTR(init);
}

// init.platform(1) # init platform within it's stage
STATIC mp_obj_t ns_init_platform(mp_obj_t self_in, mp_obj_t stage_in)
{
    ns_init_obj_t *self = MP_OBJ_TO_PTR(self_in);
    (void)self;

    int stage = mp_obj_get_int(stage_in);

    switch (stage) {
    case 1:
        platform_init_stage_one();
        break;
    case 2:
        platform_init_stage_two();
        break;
    case 3:
        platform_init_stage_three();
        break;
    default:
        nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError,
                  "ns: unknown platform init stage (%d), stages (1-3)",
                  stage));
        break;
    }

    return mp_const_none;
}

// init.clock()
STATIC mp_obj_t ns_init_clock(void)
{
    clock_init();
    return mp_const_none;
}

// init.rtimer()
STATIC mp_obj_t ns_init_rtimer(void)
{
    rtimer_init();
    return mp_const_none;
}

// init.process()
STATIC mp_obj_t ns_init_process(void)
{
    process_init();
    return mp_const_none;
}

// init.etimer()
STATIC mp_obj_t ns_init_etimer(void)
{
    process_start(&etimer_process, NULL);
    return mp_const_none;
}

// init.ctimer()
STATIC mp_obj_t ns_init_ctimer(void)
{
    ctimer_init();
    return mp_const_none;
}

// init.watchdog()
STATIC mp_obj_t ns_init_watchdog(void)
{
    watchdog_init();
    return mp_const_none;
}

// init.energest()
STATIC mp_obj_t ns_init_energest(void)
{
    energest_init();
    return mp_const_none;
}

// init.stack_check()
STATIC mp_obj_t ns_init_stack_check(void)
{
    stack_check_init();
    return mp_const_none;
}

// init.netstack()
STATIC mp_obj_t ns_init_netstack(void)
{
    netstack_init();
    return mp_const_none;
}

// init.node_id()
STATIC mp_obj_t ns_init_node_id(void)
{
    node_id_init();
    return mp_const_none;
}

// init.ipv6_addr()
STATIC mp_obj_t ns_init_ipv6_addr(void)
{
#if NETSTACK_CONF_WITH_IPV6
    uip_ds6_addr_t *lladdr;
    memcpy(&uip_lladdr.addr, &linkaddr_node_addr, sizeof(uip_lladdr.addr));
    process_start(&tcpip_process, NULL);
    lladdr = uip_ds6_get_link_local(-1);
    (void)lladdr; // TODO: print tentative link-local IPv6 address
#endif
    return mp_const_none;
}

// init.rpl_border_router()
STATIC mp_obj_t ns_init_rpl_border_router(void)
{
#if BUILD_WITH_RPL_BORDER_ROUTER
    rpl_border_router_init();
#endif
    return mp_const_none;
}

// init.orchestra()
STATIC mp_obj_t ns_init_orchestra(void)
{
#if BUILD_WITH_ORCHESTRA
    orchestra_init();
#endif
    return mp_const_none;
}

// init.shell()
STATIC mp_obj_t ns_init_shell(void)
{
#if BUILD_WITH_SHELL
    serial_shell_init();
#endif
    return mp_const_none;
}

// init.coap_engine()
STATIC mp_obj_t ns_init_coap_engine(void)
{
#if BUILD_WITH_COAP
    coap_engine_init();
#endif
    return mp_const_none;
}

// init.simple_energest()
STATIC mp_obj_t ns_init_simple_energest(void)
{
#if BUILD_WITH_SIMPLE_ENERGEST
    simple_energest_init();
#endif
    return mp_const_none;
}

// init.tsch_cs_adaptions()
STATIC mp_obj_t ns_init_tsch_cs_adaptions(void)
{
#if BUILD_WITH_TSCH_CS
    tsch_cs_adaptions_init();
#endif
    return mp_const_none;
}

// print(init)
STATIC void ns_init_print(const mp_print_t *print,
                          mp_obj_t self_in,
                          mp_print_kind_t kind)
{
    return; // TODO: print initialized information
}

STATIC MP_DEFINE_CONST_FUN_OBJ_2(ns_init_platform_obj, ns_init_platform);
STATIC MP_DEFINE_CONST_FUN_OBJ_0(ns_init_clock_obj, ns_init_clock);
STATIC MP_DEFINE_CONST_FUN_OBJ_0(ns_init_rtimer_obj, ns_init_rtimer);
STATIC MP_DEFINE_CONST_FUN_OBJ_0(ns_init_process_obj, ns_init_process);
STATIC MP_DEFINE_CONST_FUN_OBJ_0(ns_init_etimer_obj, ns_init_etimer);
STATIC MP_DEFINE_CONST_FUN_OBJ_0(ns_init_ctimer_obj, ns_init_ctimer);
STATIC MP_DEFINE_CONST_FUN_OBJ_0(ns_init_watchdog_obj, ns_init_watchdog);
STATIC MP_DEFINE_CONST_FUN_OBJ_0(ns_init_energest_obj, ns_init_energest);
STATIC MP_DEFINE_CONST_FUN_OBJ_0(ns_init_stack_check_obj, ns_init_stack_check);
STATIC MP_DEFINE_CONST_FUN_OBJ_0(ns_init_netstack_obj, ns_init_netstack);
STATIC MP_DEFINE_CONST_FUN_OBJ_0(ns_init_node_id_obj, ns_init_node_id);
STATIC MP_DEFINE_CONST_FUN_OBJ_0(ns_init_ipv6_addr_obj, ns_init_ipv6_addr);
STATIC MP_DEFINE_CONST_FUN_OBJ_0(ns_init_rpl_border_router_obj, ns_init_rpl_border_router);
STATIC MP_DEFINE_CONST_FUN_OBJ_0(ns_init_orchestra_obj, ns_init_orchestra);
STATIC MP_DEFINE_CONST_FUN_OBJ_0(ns_init_shell_obj, ns_init_shell);
STATIC MP_DEFINE_CONST_FUN_OBJ_0(ns_init_coap_engine_obj, ns_init_coap_engine);
STATIC MP_DEFINE_CONST_FUN_OBJ_0(ns_init_simple_energest_obj, ns_init_simple_energest);
STATIC MP_DEFINE_CONST_FUN_OBJ_0(ns_init_tsch_cs_adaptions_obj, ns_init_tsch_cs_adaptions);

STATIC const mp_rom_map_elem_t ns_init_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_platform), MP_ROM_PTR(&ns_init_platform_obj) },
    { MP_ROM_QSTR(MP_QSTR_clock), MP_ROM_PTR(&ns_init_clock_obj) },
    { MP_ROM_QSTR(MP_QSTR_rtimer), MP_ROM_PTR(&ns_init_rtimer_obj) },
    { MP_ROM_QSTR(MP_QSTR_process), MP_ROM_PTR(&ns_init_process_obj) },
    { MP_ROM_QSTR(MP_QSTR_etimer), MP_ROM_PTR(&ns_init_etimer_obj) },
    { MP_ROM_QSTR(MP_QSTR_ctimer), MP_ROM_PTR(&ns_init_ctimer_obj) },
    { MP_ROM_QSTR(MP_QSTR_watchdog), MP_ROM_PTR(&ns_init_watchdog_obj) },
    { MP_ROM_QSTR(MP_QSTR_energest), MP_ROM_PTR(&ns_init_energest_obj) },
    { MP_ROM_QSTR(MP_QSTR_stack_check), MP_ROM_PTR(&ns_init_stack_check_obj) },
    { MP_ROM_QSTR(MP_QSTR_netstack), MP_ROM_PTR(&ns_init_netstack_obj) },
    { MP_ROM_QSTR(MP_QSTR_node_id), MP_ROM_PTR(&ns_init_node_id_obj) },
    { MP_ROM_QSTR(MP_QSTR_ipv6_addr), MP_ROM_PTR(&ns_init_ipv6_addr_obj) },
    { MP_ROM_QSTR(MP_QSTR_rpl_border_router), MP_ROM_PTR(&ns_init_rpl_border_router_obj) },
    { MP_ROM_QSTR(MP_QSTR_orchestra), MP_ROM_PTR(&ns_init_orchestra_obj) },
    { MP_ROM_QSTR(MP_QSTR_shell), MP_ROM_PTR(&ns_init_shell_obj) },
    { MP_ROM_QSTR(MP_QSTR_coap_engine), MP_ROM_PTR(&ns_init_coap_engine_obj) },
    { MP_ROM_QSTR(MP_QSTR_simple_energest), MP_ROM_PTR(&ns_init_simple_energest_obj) },
    { MP_ROM_QSTR(MP_QSTR_tsch_cs_adaptions), MP_ROM_PTR(&ns_init_tsch_cs_adaptions_obj) },
};

STATIC MP_DEFINE_CONST_DICT(ns_init_locals_dict, ns_init_locals_dict_table);

const mp_obj_type_t ns_init_type = {
    { &mp_type_type },
    .name = MP_QSTR_Init,
    .print = ns_init_print,
    .make_new = ns_init_make_new,
    .locals_dict = (mp_obj_dict_t *)&ns_init_locals_dict,
};
