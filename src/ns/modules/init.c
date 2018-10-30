#include "py/nlr.h"
#include "py/runtime.h"
#include "ns/contiki.h"
#include "ns/contiki-net.h"
#include "ns/sys/node-id.h"
#include "ns/sys/platform.h"
#include "ns/sys/energest.h"
#if !defined(UNIX)
#include "ns/sys/stack-check.h"
#endif
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
//
//      init.stack_check()          # this is not available in unix build and
//                                    will raise exception if call this function
//
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
static uip_ds6_addr_t *lladdr;

typedef struct _ns_init_obj_t {
    mp_obj_base_t base;
    bool use_platform[3];
    bool use_clock;
    bool use_rtimer;
    bool use_process;
    bool use_etimer;
    bool use_ctimer;
    bool use_watchdog;
    bool use_energest;
    bool use_stack_check;
    bool use_netstack;
    bool use_node_id;
    bool use_ipv6_addr;
    bool use_rpl_border_router;
    bool use_orchestra;
    bool use_shell;
    bool use_coap_engine;
    bool use_simple_energest;
    bool use_tsch_cs_adaptions;
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

    int stage = mp_obj_get_int(stage_in);

    switch (stage) {
    case 1:
        platform_init_stage_one();
        self->use_platform[stage - 1] = true;
        break;
    case 2:
        platform_init_stage_two();
        self->use_platform[stage - 1] = true;
        break;
    case 3:
        platform_init_stage_three();
        self->use_platform[stage - 1] = true;
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
STATIC mp_obj_t ns_init_clock(mp_obj_t self_in)
{
    ns_init_obj_t *self = MP_OBJ_TO_PTR(self_in);
    clock_init();
    self->use_clock = true;
    return mp_const_none;
}

// init.rtimer()
STATIC mp_obj_t ns_init_rtimer(mp_obj_t self_in)
{
    ns_init_obj_t *self = MP_OBJ_TO_PTR(self_in);
    rtimer_init();
    self->use_rtimer = true;
    return mp_const_none;
}

// init.process()
STATIC mp_obj_t ns_init_process(mp_obj_t self_in)
{
    ns_init_obj_t *self = MP_OBJ_TO_PTR(self_in);
    process_init();
    self->use_process = true;
    return mp_const_none;
}

// init.etimer()
STATIC mp_obj_t ns_init_etimer(mp_obj_t self_in)
{
    ns_init_obj_t *self = MP_OBJ_TO_PTR(self_in);
    process_start(&etimer_process, NULL);
    self->use_etimer = true;
    return mp_const_none;
}

// init.ctimer()
STATIC mp_obj_t ns_init_ctimer(mp_obj_t self_in)
{
    ns_init_obj_t *self = MP_OBJ_TO_PTR(self_in);
    ctimer_init();
    self->use_ctimer = true;
    return mp_const_none;
}

// init.watchdog()
STATIC mp_obj_t ns_init_watchdog(mp_obj_t self_in)
{
    ns_init_obj_t *self = MP_OBJ_TO_PTR(self_in);
    watchdog_init();
    self->use_watchdog = true;
    return mp_const_none;
}

// init.energest()
STATIC mp_obj_t ns_init_energest(mp_obj_t self_in)
{
    ns_init_obj_t *self = MP_OBJ_TO_PTR(self_in);
    energest_init();
    self->use_energest = true;
    return mp_const_none;
}

// init.stack_check() # this is not available in unix build and will raise exception
STATIC mp_obj_t ns_init_stack_check(mp_obj_t self_in)
{
#if !defined(UNIX)
    ns_init_obj_t *self = MP_OBJ_TO_PTR(self_in);
    stack_check_init();
    self->use_stack_check = true;
#else
    nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError,
                "ns: this feature not available in unix build"));
#endif
    return mp_const_none;
}

// init.netstack()
STATIC mp_obj_t ns_init_netstack(mp_obj_t self_in)
{
    ns_init_obj_t *self = MP_OBJ_TO_PTR(self_in);
    netstack_init();
    self->use_netstack = true;
    return mp_const_none;
}

// init.node_id()
STATIC mp_obj_t ns_init_node_id(mp_obj_t self_in)
{
    ns_init_obj_t *self = MP_OBJ_TO_PTR(self_in);
    node_id_init();
    self->use_node_id = true;
    return mp_const_none;
}

// init.ipv6_addr()
STATIC mp_obj_t ns_init_ipv6_addr(mp_obj_t self_in)
{
#if NETSTACK_CONF_WITH_IPV6
    ns_init_obj_t *self = MP_OBJ_TO_PTR(self_in);
    memcpy(&uip_lladdr.addr, &linkaddr_node_addr, sizeof(uip_lladdr.addr));
    process_start(&tcpip_process, NULL);
    lladdr = uip_ds6_get_link_local(-1);
    self->use_ipv6_addr = true;
#endif
    return mp_const_none;
}

// init.rpl_border_router()
STATIC mp_obj_t ns_init_rpl_border_router(mp_obj_t self_in)
{
#if BUILD_WITH_RPL_BORDER_ROUTER
    ns_init_obj_t *self = MP_OBJ_TO_PTR(self_in);
    rpl_border_router_init();
    self->use_rpl_border_router = true;
#endif
    return mp_const_none;
}

// init.orchestra()
STATIC mp_obj_t ns_init_orchestra(mp_obj_t self_in)
{
#if BUILD_WITH_ORCHESTRA
    ns_init_obj_t *self = MP_OBJ_TO_PTR(self_in);
    orchestra_init();
    self->use_orchestra = true;
#endif
    return mp_const_none;
}

// init.shell()
STATIC mp_obj_t ns_init_shell(mp_obj_t self_in)
{
#if BUILD_WITH_SHELL
    ns_init_obj_t *self = MP_OBJ_TO_PTR(self_in);
    serial_shell_init();
    self->use_shell = true;
#endif
    return mp_const_none;
}

// init.coap_engine()
STATIC mp_obj_t ns_init_coap_engine(mp_obj_t self_in)
{
#if BUILD_WITH_COAP
    ns_init_obj_t *self = MP_OBJ_TO_PTR(self_in);
    coap_engine_init();
    self->use_coap_engine = true;
#endif
    return mp_const_none;
}

// init.simple_energest()
STATIC mp_obj_t ns_init_simple_energest(mp_obj_t self_in)
{
#if BUILD_WITH_SIMPLE_ENERGEST
    ns_init_obj_t *self = MP_OBJ_TO_PTR(self_in);
    simple_energest_init();
    self->use_simple_energest = true;
#endif
    return mp_const_none;
}

// init.tsch_cs_adaptions()
STATIC mp_obj_t ns_init_tsch_cs_adaptions(mp_obj_t self_in)
{
#if BUILD_WITH_TSCH_CS
    ns_init_obj_t *self = MP_OBJ_TO_PTR(self_in);
    tsch_cs_adaptions_init();
    self->use_tsch_cs_adaptions = true;
#endif
    return mp_const_none;
}

// print(init)
STATIC void ns_init_print(const mp_print_t *print,
                          mp_obj_t self_in,
                          mp_print_kind_t kind)
{
    ns_init_obj_t *self = MP_OBJ_TO_PTR(self_in);

    mp_printf(print, "ns: --- Nespy init modules ----\n");
    mp_printf(print, "ns: platform stage [1]    : %s\n", self->use_platform[0] ? "true" : "false");
    mp_printf(print, "ns: platform stage [2]    : %s\n", self->use_platform[1] ? "true" : "false");
    mp_printf(print, "ns: platform stage [3]    : %s\n", self->use_platform[2] ? "true" : "false");
    mp_printf(print, "ns: clock                 : %s\n", self->use_clock ? "true" : "false");
    mp_printf(print, "ns: rtimer                : %s\n", self->use_rtimer ? "true" : "false");
    mp_printf(print, "ns: process               : %s\n", self->use_process ? "true" : "false");
    mp_printf(print, "ns: etimer                : %s\n", self->use_etimer ? "true" : "false");
    mp_printf(print, "ns: ctimer                : %s\n", self->use_ctimer ? "true" : "false");
    mp_printf(print, "ns: watchdog              : %s\n", self->use_watchdog ? "true" : "false");
    mp_printf(print, "ns: energest              : %s\n", self->use_energest ? "true" : "false");
#if !defined(UNIX)
    mp_printf(print, "ns: stack check           : %s\n", self->use_clock ? "true" : "false");
#else
    mp_printf(print, "ns: stack check           : not-supported\n");
#endif
    mp_printf(print, "ns: netstack              : %s\n", self->use_netstack ? "true" : "false");
    mp_printf(print, "ns: node id               : %s\n", self->use_node_id ? "true" : "false");
    mp_printf(print, "ns: ipv6 address          : %s\n", self->use_ipv6_addr ? "true" : "false");
    mp_printf(print, "ns: rpl border router     : %s\n", self->use_rpl_border_router ? "true" : "false");
    mp_printf(print, "ns: orchestra             : %s\n", self->use_orchestra ? "true" : "false");
    mp_printf(print, "ns: shell                 : %s\n", self->use_shell ? "true" : "false");
    mp_printf(print, "ns: coap engine           : %s\n", self->use_coap_engine ? "true" : "false");
    mp_printf(print, "ns: simple energest       : %s\n", self->use_simple_energest ? "true" : "false");
    mp_printf(print, "ns: tsch cs adaptions     : %s\n", self->use_tsch_cs_adaptions ? "true" : "false");
    mp_printf(print, "ns: --- Nespy network stack ---\n");
    mp_printf(print, "ns: Routing: %s\n", NETSTACK_ROUTING.name);
    mp_printf(print, "ns: Net: %s\n", NETSTACK_NETWORK.name);
    mp_printf(print, "ns: MAC: %s\n", NETSTACK_MAC.name);
    mp_printf(print, "ns: 802.15.4 PANID: 0x%04x\n", IEEE802154_PANID);
#if MAC_CONF_WITH_TSCH
    mp_printf(print, "ns: 802.15.4 TSCH default hopping sequence length: %u\n",
              (unsigned)sizeof(TSCH_DEFAULT_HOPPING_SEQUENCE));
#else
    mp_printf(print, "ns: 802.15.4 Default channel: %u\n", IEEE802154_DEFAULT_CHANNEL);
#endif
    mp_printf(print, "ns: Node ID: %u\n", node_id);
    mp_printf(print, "ns: Link-layer address: ");

    linkaddr_t *laddr = &linkaddr_node_addr;

    if (laddr == NULL) {
        mp_printf(print, "(NULL LL addr)");
    } else {
        unsigned i;
        for (i = 0; i < LINKADDR_SIZE; i++) {
            if (i > 0 && i % 2 == 0) {
                mp_printf(print, ".");
            }
            mp_printf(print, "%02x", laddr->u8[i]);
        }
    }

#if NETSTACK_CONF_WITH_IPV6
    mp_printf(print, "\n");
    // print ipv6 address
    char buf[UIPLIB_IPV6_MAX_STR_LEN];
    uiplib_ipaddr_snprint(buf, sizeof(buf), lladdr != NULL ? &lladdr->ipaddr : NULL);
    mp_printf(print, "ns: Tentative link-local IPv6 address: %s", buf);
#endif
}

STATIC MP_DEFINE_CONST_FUN_OBJ_2(ns_init_platform_obj, ns_init_platform);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(ns_init_clock_obj, ns_init_clock);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(ns_init_rtimer_obj, ns_init_rtimer);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(ns_init_process_obj, ns_init_process);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(ns_init_etimer_obj, ns_init_etimer);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(ns_init_ctimer_obj, ns_init_ctimer);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(ns_init_watchdog_obj, ns_init_watchdog);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(ns_init_energest_obj, ns_init_energest);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(ns_init_stack_check_obj, ns_init_stack_check);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(ns_init_netstack_obj, ns_init_netstack);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(ns_init_node_id_obj, ns_init_node_id);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(ns_init_ipv6_addr_obj, ns_init_ipv6_addr);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(ns_init_rpl_border_router_obj, ns_init_rpl_border_router);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(ns_init_orchestra_obj, ns_init_orchestra);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(ns_init_shell_obj, ns_init_shell);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(ns_init_coap_engine_obj, ns_init_coap_engine);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(ns_init_simple_energest_obj, ns_init_simple_energest);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(ns_init_tsch_cs_adaptions_obj, ns_init_tsch_cs_adaptions);

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
