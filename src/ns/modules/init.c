#include "py/nlr.h"
#include "py/runtime.h"
#include "ns/contiki.h"
#include "ns/contiki-net.h"
#include "ns/sys/node-id.h"
#include "ns/sys/platform.h"
#if !defined(UNIX)
#include "ns/sys/stack-check.h"
#endif
#include "ns/net/app-layer/coap/coap-engine.h"
#include "ns/services/rpl-border-router/rpl-border-router.h"
#include "ns/net/mac/framer/framer-802154.h"
#include "ns/services/tsch-cs/tsch-cs.h"
#include "ns/modules/cli/cli.h"
#include "ns/modules/cli/cli-uart.h"

#include <stdint.h>
#include <stdio.h>

// Example usage to Init objects
//
//      init = ns.Init()
//
//      # mandatory init
//      init.node_id(2)
//      init.netstack()
//      init.platform()             # init platform stage three
//
//      # optional init
//      init.stack_check()
//      init.rpl_border_router()
//      init.cli()
//      init.coap_engine()
//      init.tsch_cs_adaptions()

const mp_obj_type_t ns_init_type;
static uip_ds6_addr_t *lladdr;

typedef struct _ns_init_obj_t {
    mp_obj_base_t base;
} ns_init_obj_t;

static bool is_init_obj_created = false;

#if defined(UNIX)
extern uint16_t unix_radio_get_port(void);
#endif

// init = ns.Init() # init object constructor
STATIC mp_obj_t ns_init_make_new(const mp_obj_type_t *type,
                                 size_t n_args,
                                 size_t n_kw,
                                 const mp_obj_t *all_args)
{
    // check arguments
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

// init.node_id(2)
STATIC mp_obj_t ns_init_node_id(mp_obj_t self_in, mp_obj_t id_in)
{
    int id = mp_obj_get_int(id_in);
    node_id_init();
    node_id_set(id);
    return mp_const_none;
}

// init.netstack()
STATIC mp_obj_t ns_init_netstack(mp_obj_t self_in)
{
    platform_init_stage_one(); // boot up related init
    clock_init();
    rtimer_init();
    process_init();
    process_start(&etimer_process, NULL);
    ctimer_init();
    platform_init_stage_two(); // netstack related init
#if NETSTACK_CONF_WITH_IPV6
    memcpy(&uip_lladdr.addr, &linkaddr_node_addr, sizeof(uip_lladdr.addr));
    process_start(&tcpip_process, NULL);
    lladdr = uip_ds6_get_link_local(-1);
#endif
    netstack_init();
#if APP_CONF_WITH_COAP
    coap_engine_init();
#endif
    return mp_const_none;
}

// init.platform()
STATIC mp_obj_t ns_init_platform(mp_obj_t self_in)
{
    platform_init_stage_three(); // low-level driver or sensor related init
    return mp_const_none;
}

// init.stack_check()
STATIC mp_obj_t ns_init_stack_check(mp_obj_t self_in)
{
#if !defined(UNIX)
    stack_check_init();
#else
    nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError,
                "ns: this feature not available in unix build"));
#endif
    return mp_const_none;
}

// init.rpl_border_router()
STATIC mp_obj_t ns_init_rpl_border_router(mp_obj_t self_in)
{
#if BUILD_WITH_RPL_BORDER_ROUTER
    rpl_border_router_init();
#endif
    return mp_const_none;
}

// init.cli()
STATIC mp_obj_t ns_init_cli(mp_obj_t self_in)
{
    cli_commands_init();
    cli_uart_init();
    return mp_const_none;
}

// init.coap_engine()
STATIC mp_obj_t ns_init_coap_engine(mp_obj_t self_in)
{
#if BUILD_WITH_COAP
    coap_engine_init();
#endif
    return mp_const_none;
}

// init.tsch_cs_adaptions()
STATIC mp_obj_t ns_init_tsch_cs_adaptions(mp_obj_t self_in)
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
    mp_printf(print, "ns: --- Nespy network stack ---\n");
    mp_printf(print, "ns: Routing: %s\n", NETSTACK_ROUTING.name);
    mp_printf(print, "ns: Net: %s\n", NETSTACK_NETWORK.name);
    mp_printf(print, "ns: MAC: %s\n", NETSTACK_MAC.name);
#if defined(UNIX)
    mp_printf(print, "ns: Radio PORT: %u\n", unix_radio_get_port());
#endif
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
    mp_printf(print, "ns: Tentative link-local IPv6 address: %s\n", buf);
#endif
}

STATIC MP_DEFINE_CONST_FUN_OBJ_2(ns_init_node_id_obj, ns_init_node_id);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(ns_init_netstack_obj, ns_init_netstack);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(ns_init_platform_obj, ns_init_platform);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(ns_init_stack_check_obj, ns_init_stack_check);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(ns_init_rpl_border_router_obj, ns_init_rpl_border_router);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(ns_init_cli_obj, ns_init_cli);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(ns_init_coap_engine_obj, ns_init_coap_engine);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(ns_init_tsch_cs_adaptions_obj, ns_init_tsch_cs_adaptions);

STATIC const mp_rom_map_elem_t ns_init_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_node_id), MP_ROM_PTR(&ns_init_node_id_obj) },
    { MP_ROM_QSTR(MP_QSTR_netstack), MP_ROM_PTR(&ns_init_netstack_obj) },
    { MP_ROM_QSTR(MP_QSTR_platform), MP_ROM_PTR(&ns_init_platform_obj) },
    { MP_ROM_QSTR(MP_QSTR_stack_check), MP_ROM_PTR(&ns_init_stack_check_obj) },
    { MP_ROM_QSTR(MP_QSTR_rpl_border_router), MP_ROM_PTR(&ns_init_rpl_border_router_obj) },
    { MP_ROM_QSTR(MP_QSTR_cli), MP_ROM_PTR(&ns_init_cli_obj) },
    { MP_ROM_QSTR(MP_QSTR_coap_engine), MP_ROM_PTR(&ns_init_coap_engine_obj) },
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
