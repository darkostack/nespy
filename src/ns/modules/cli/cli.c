#include "ns/contiki.h"
#include "ns/contiki-net.h"
#include "ns/sys/node-id.h"
#include "ns/lib/random.h"

#if APP_CONF_WITH_COAP
#include "ns/net/app-layer/coap/coap-engine.h"
#include "ns/net/app-layer/coap/coap-blocking-api.h"
#endif

#include "ns/net/netstack.h"
#include "ns/net/ipv6/uip.h"
#include "ns/net/ipv6/uiplib.h"
#include "ns/net/ipv6/uip-icmp6.h"
#include "ns/net/ipv6/uip-ds6.h"
#if MAC_CONF_WITH_TSCH
#include "ns/net/mac/tsch/tsch.h"
#endif /* MAC_CONF_WITH_TSCH */
#include "ns/net/routing/routing.h"
#include "ns/net/mac/llsec802154.h"

/* For RPL-specific commands */
#if ROUTING_CONF_RPL_LITE
#include "net/routing/rpl-lite/rpl.h"
#elif ROUTING_CONF_RPL_CLASSIC
#include "net/routing/rpl-classic/rpl.h"
#endif

#include "ns/modules/cli/cli.h"
#include "ns/modules/cli/cli-uart.h"
#include "ns/modules/nstd.h"

#include "genhdr/mpversion.h"
#include <string.h>

#define PING_TIMEOUT (3 * CLOCK_SECOND)

static struct uip_icmp6_echo_reply_notification echo_reply_notification;
static uint8_t curr_ping_ttl;
static uint16_t curr_ping_datalen;
static unsigned long curr_ping_times;
static unsigned long curr_ping_counter;
static uip_ipaddr_t ping_remote_addr;
#if APP_CONF_WITH_COAP
static coap_endpoint_t server_ep;
static char server_ep_buf[64];
static coap_message_t request[1];
static coap_observee_t *obs;
static uip_ipaddr_t coap_ep_ipaddr;
#endif

PROCESS(cli_ping_process, "Cli-ping process");
#if APP_CONF_WITH_COAP
PROCESS(ns_coap_client, "Nespy coap client");
PROCESS_NAME(ns_coap_server);
#endif

#if defined(UNIX)
extern uint16_t unix_radio_get_port(void);
#endif

// commands
static void command_help(int argc, char *argv[]);
static void command_ps(int argc, char *argv[]);
static void command_version(int argc, char *argv[]);
static void command_ipaddr(int argc, char *argv[]);
static void command_ip_neighbors(int argc, char *argv[]);
#if UIP_CONF_IPV6_RPL
static void command_rpl_set_root(int argc, char *argv[]);
static void command_rpl_local_repair(int argc, char *argv[]);
#if ROUTING_CONF_RPL_LITE
static void command_rpl_refresh_routes(int argc, char *argv[]);
#endif // ROUTING_CONF_RPL_LITE
static void command_rpl_global_repair(int argc, char *argv[]);
#endif // UIP_CONF_IPV6_RPL
#if ROUTING_CONF_RPL_LITE
static void command_rpl_status(int argc, char *argv[]);
static void command_rpl_nbr(int argc, char *argv[]);
#endif // ROUTING_CONF_RPL_LITE
static void command_routes(int argc, char *argv[]);
static void command_ping(int argc, char *argv[]);
#if defined(UNIX)
static void command_exit(int argc, char *argv[]);
#endif
#if APP_CONF_WITH_COAP
static void command_coap_server_start(int argc, char *argv[]);
static void command_coap_client_ep(int argc, char *argv[]);
static void command_coap_client_obs(int argc, char *argv[]);
static void command_coap_client_get(int argc, char *argv[]);
#endif

// helper function
static void cli_output_6addr(const uip_ipaddr_t *ipaddr);
static void cli_output_lladdr(const linkaddr_t *lladdr);
static const char * ds6_nbr_state_to_str(uint8_t state);
static const char * rpl_state_to_str(enum rpl_dag_state state);
static const char *rpl_mop_to_str(int mop);
static const char *rpl_ocp_to_str(int ocp);
static void echo_reply_handler(uip_ipaddr_t *source, uint8_t ttl, uint8_t *data, uint16_t datalen);

static const ns_cli_cmd_t s_commands[] = {
    { "help", "get list of available commands", &command_help },
    { "ps", "get list of current running process", &command_ps },
    { "version", "get nespy version and netstack config", &command_version },
    { "ip-addr", "get node IPv6 address", &command_ipaddr },
    { "ip-nbr", "get node neighbor address", &command_ip_neighbors },
#if UIP_CONF_IPV6_RPL
    { "rpl-set-root", "set node as a root", &command_rpl_set_root },
    { "rpl-local-repair", "trigger rpl local repair", &command_rpl_local_repair },
#if ROUTING_CONF_RPL_LITE
    { "rpl-refresh-routes", "refresh rpl routes", &command_rpl_refresh_routes },
#endif // ROUTING_CONF_RPL_LITE
    { "rpl-global-repair", "trigger rpl global repair", &command_rpl_global_repair },
#endif // UIP_CONF_IPV6_RPL
#if ROUTING_CONF_RPL_LITE
    { "rpl-status", "get rpl status", &command_rpl_status },
    { "rpl-nbr", "get rpl neighbor", &command_rpl_nbr },
#endif // ROUTING_CONF_RPL_LITE
    { "routes", "get node routes", &command_routes },
    { "ping", "IPv6 ping command", &command_ping },
#if defined(UNIX)
    { "exit", "exit unix program", &command_exit },
#endif
#if APP_CONF_WITH_COAP
    { "coap-server-start", "set this node as coap server", &command_coap_server_start },
    { "coap-client-ep", "set server end-point", &command_coap_client_ep }, // set coap client end-point IPv6 address
    { "coap-client-obs", "set coap server observation", &command_coap_client_obs },
    { "coap-client-get", "send coap GET command", &command_coap_client_get },
#endif
};

static void command_help(int argc, char *argv[])
{
    cli_uart_output_format("%-20s %s\r\n", "Command", "Description");
    cli_uart_output_format("---------------------------------------\r\n");
    for (unsigned int i = 0; i < (sizeof(s_commands) / sizeof(s_commands[0])); i++) {
        cli_uart_output_format("%-20s %s\r\n", s_commands[i].name, s_commands[i].desc);
    }
}

static void command_ps(int argc, char *argv[])
{
    struct process *p;
    cli_uart_output_format("Process list:\r\n");
    for (p = process_list; p != NULL; p = p->next) {
        cli_uart_output_format("-- %s\r\n", p->name);
    }
}

static void command_version(int argc, char *argv[])
{
    cli_uart_output_format("Nespy v%s %s : build-date %s\r\n",
                           MICROPY_VERSION_STRING,
                           MICROPY_GIT_TAG,
                           MICROPY_BUILD_DATE);
    cli_uart_output_format("-- Routing: %s\r\n", NETSTACK_ROUTING.name);
    cli_uart_output_format("-- Net: %s\r\n", NETSTACK_NETWORK.name);
    cli_uart_output_format("-- MAC: %s\r\n", NETSTACK_MAC.name);
    cli_uart_output_format("-- 802.15.4 PANID: 0x%04x\r\n", IEEE802154_PANID);
#if defined(UNIX)
    cli_uart_output_format("-- Radio PORT: %u\r\n", unix_radio_get_port());
#endif
#if MAC_CONF_WITH_TSCH
    cli_uart_output_format("-- 802.15.4 TSCH default hopping sequence length: %u\r\n",
              (unsigned)sizeof(TSCH_DEFAULT_HOPPING_SEQUENCE));
#else
    cli_uart_output_format("-- 802.15.4 Default channel: %u\r\n", IEEE802154_DEFAULT_CHANNEL);
#endif
    cli_uart_output_format("-- Node ID: %u\r\n", node_id);
    cli_uart_output_format("-- Link-layer address: ");

    linkaddr_t *laddr = &linkaddr_node_addr;

    if (laddr == NULL) {
        cli_uart_output_format("(NULL LL addr)");
    } else {
        unsigned i;
        for (i = 0; i < LINKADDR_SIZE; i++) {
            if (i > 0 && i % 2 == 0) {
                cli_uart_output_format(".");
            }
            cli_uart_output_format("%02x", laddr->u8[i]);
        }
    }
    cli_uart_output_format("\r\n");
#if NETSTACK_CONF_WITH_IPV6
    // print ipv6 address
    char buf[UIPLIB_IPV6_MAX_STR_LEN];
    uip_ds6_addr_t *lladdr = uip_ds6_get_link_local(-1);
    uiplib_ipaddr_snprint(buf, sizeof(buf), lladdr != NULL ? &lladdr->ipaddr : NULL);
    cli_uart_output_format("-- Tentative link-local IPv6 address: %s\r\n", buf);
#endif
}

static void command_ipaddr(int argc, char *argv[])
{
    uint8_t state;
    cli_uart_output_format("Node IPv6 addresses:\r\n");
    for (int i = 0; i < UIP_DS6_ADDR_NB; i++) {
        state = uip_ds6_if.addr_list[i].state;
        if (uip_ds6_if.addr_list[i].isused &&
            (state == ADDR_TENTATIVE || state == ADDR_PREFERRED)) {
            cli_uart_output_format("-- ");
            cli_output_6addr(&uip_ds6_if.addr_list[i].ipaddr);
            cli_uart_output_format("\r\n");
        }
    }
}

static void command_ip_neighbors(int argc, char *argv[])
{
    uip_ds6_nbr_t *nbr;
    nbr = uip_ds6_nbr_head();
    if (nbr == NULL) {
        cli_uart_output_format("Node IPv6 neighbors: none\r\n");
        return;
    }
    cli_uart_output_format("Node IPv6 neighbors:\r\n");
    while (nbr != NULL) {
        cli_uart_output_format("-- ");
        cli_output_6addr(uip_ds6_nbr_get_ipaddr(nbr));
        cli_uart_output_format(" <-> ");
        cli_output_lladdr((linkaddr_t *)uip_ds6_nbr_get_ll(nbr));
        cli_uart_output_format(", router %u, state %s ",
                nbr->isrouter, ds6_nbr_state_to_str(nbr->state));
        cli_uart_output_format("\r\n");
        nbr = uip_ds6_nbr_next(nbr);
    }
}

#if UIP_CONF_IPV6_RPL
static void command_rpl_set_root(int argc, char *argv[])
{
    static int is_on;
    static uip_ipaddr_t prefix;

    if (argc == 0) {
        cli_uart_output_format("not enough arguments\r\n");
        return;
    }

    // get enable status
    if (ns_strcmp(argv[0], "1") == 0) {
        is_on = 1;
    } else if (ns_strcmp(argv[0], "0") == 0) {
        is_on = 0;
    } else {
        cli_uart_output_format("invalid argument: %s\r\n", argv[0]);
        return;
    }

    // set prefix if it assigned, default fd00::
    if (argc == 2) {
        if (uiplib_ipaddrconv(argv[1], &prefix) == 0) {
            cli_uart_output_format("Invalid prefix: %s\r\n", argv[1]);
            return;
        }
    } else {
        uip_ip6addr(&prefix, UIP_DS6_DEFAULT_PREFIX, 0, 0, 0, 0, 0, 0, 0);
    }

    if (is_on) {
        if (!NETSTACK_ROUTING.node_is_root()) {
            cli_uart_output_format("Setting as DAG root with prefix ");
            cli_output_6addr(&prefix);
            cli_uart_output_format("/64\r\n");
            NETSTACK_ROUTING.root_set_prefix(&prefix, NULL);
            NETSTACK_ROUTING.root_start();
        } else {
            cli_uart_output_format("Node is already a DAG root\r\n");
        }
    } else {
        if (NETSTACK_ROUTING.node_is_root()) {
            cli_uart_output_format("Setting as non-root node: leaving DAG\r\n");
            NETSTACK_ROUTING.leave_network();
        } else {
            cli_uart_output_format("Node is not a DAG root\r\n");
        }
    }
}

static void command_rpl_local_repair(int argc, char *argv[])
{
    cli_uart_output_format("Triggering routing local repair\r\n");
    NETSTACK_ROUTING.local_repair("Cli");
}

#if ROUTING_CONF_RPL_LITE
static void command_rpl_refresh_routes(int argc, char *argv[])
{
    cli_uart_output_format("Triggering routes refresh\r\n");
    rpl_refresh_routes("Cli");
}
#endif // ROUTING_CONF_RPL_LITE

static void command_rpl_global_repair(int argc, char *argv[])
{
    cli_uart_output_format("Triggering routing global repair\r\n");
    NETSTACK_ROUTING.global_repair("Cli");
}
#endif // UIP_CONF_IPV6_RPL

#if ROUTING_CONF_RPL_LITE
static void command_rpl_status(int argc, char *argv[])
{
    cli_uart_output_format("RPL status:\r\n");
    if (!curr_instance.used) {
        cli_uart_output_format("-- Instance: none\r\n");
    } else {
        cli_uart_output_format("-- Instance: %u\r\n", curr_instance.instance_id);
        if (NETSTACK_ROUTING.node_is_root()) {
            cli_uart_output_format("-- DAG root\r\n");
        } else {
            cli_uart_output_format("-- DAG node\r\n");
        }
        cli_uart_output_format("-- DAG: ");
        cli_output_6addr(&curr_instance.dag.dag_id);
        cli_uart_output_format(", version %u\r\n", curr_instance.dag.version);
        cli_uart_output_format("-- Prefix: ");
        cli_output_6addr(&curr_instance.dag.prefix_info.prefix);
        cli_uart_output_format("/%u\r\n", curr_instance.dag.prefix_info.length);
        cli_uart_output_format("-- MOP: %s\r\n", rpl_mop_to_str(curr_instance.mop));
        cli_uart_output_format("-- OF: %s\r\n", rpl_ocp_to_str(curr_instance.of->ocp));
        cli_uart_output_format("-- Hop rank increment: %u\r\n", curr_instance.min_hoprankinc);
        cli_uart_output_format("-- Default lifetime: %lu seconds\r\n",
                               RPL_LIFETIME(curr_instance.default_lifetime));
        cli_uart_output_format("-- State: %s\r\n", rpl_state_to_str(curr_instance.dag.state));
        cli_uart_output_format("-- Preferred parent: ");
        if (curr_instance.dag.preferred_parent) {
            cli_output_6addr(rpl_neighbor_get_ipaddr(curr_instance.dag.preferred_parent));
            cli_uart_output_format(" (last DTSN: %u)\r\n", curr_instance.dag.preferred_parent->dtsn);
        } else {
            cli_uart_output_format("none\r\n");
        }
        cli_uart_output_format("-- Rank: %u\r\n", curr_instance.dag.rank);
        cli_uart_output_format("-- Lowest rank: %u (%u)\r\n",
                               curr_instance.dag.lowest_rank, curr_instance.max_rankinc);
        cli_uart_output_format("-- DTSN out: %u\r\n", curr_instance.dtsn_out);
        cli_uart_output_format("-- DAO sequence: last sent %u, last acked %u\r\n",
                               curr_instance.dag.dao_last_seqno, curr_instance.dag.dao_last_acked_seqno);
        cli_uart_output_format("-- Trickle timer: current %u, min %u, max %u, redundancy %u\r\n",
                               curr_instance.dag.dio_intcurrent, curr_instance.dio_intmin,
                               curr_instance.dio_intmin + curr_instance.dio_intdoubl,
                               curr_instance.dio_redundancy);
    }
}

static void command_rpl_nbr(int argc, char *argv[])
{
    if (!curr_instance.used || rpl_neighbor_count() == 0) {
        cli_uart_output_format("RPL neighbors: none\r\n");
    } else {
        rpl_nbr_t *nbr = nbr_table_head(rpl_neighbors);
        cli_uart_output_format("RPL neighbors:\r\n");
        while (nbr != NULL) {
            char buf[120];
            rpl_neighbor_snprint(buf, sizeof(buf), nbr);
            cli_uart_output_format("%s\r\n", buf);
            nbr = nbr_table_next(rpl_neighbors, nbr);
        }
    }
}
#endif // ROUTING_CONF_RPL_LITE

static void command_routes(int argc, char *argv[])
{
    uip_ds6_defrt_t *default_route;

    cli_uart_output_format("Default route:\r\n");
    default_route = uip_ds6_defrt_lookup(uip_ds6_defrt_choose());
    if (default_route != NULL) {
        cli_uart_output_format("-- ");
        cli_output_6addr(&default_route->ipaddr);
        if (default_route->lifetime.interval != 0) {
            cli_uart_output_format(" (lifetime: %lu seconds)\r\n",
                    (unsigned long)default_route->lifetime.interval);
        } else {
            cli_uart_output_format(" (lifetime: infinite)\r\n");
        }
    } else {
        cli_uart_output_format("-- none\r\n");
    }

#if UIP_CONF_IPV6_RPL
    if (uip_sr_num_nodes() > 0) {
        uip_sr_node_t *link;
        cli_uart_output_format("Routing links (%u in total):\r\n", uip_sr_num_nodes());
        link = uip_sr_node_head();
        while (link != NULL) {
            char buf[100];
            uip_sr_link_snprint(buf, sizeof(buf), link);
            cli_uart_output_format("-- %s\r\n", buf);
            link = uip_sr_node_next(link);
        }
    } else {
        cli_uart_output_format("No routing links\r\n");
    }
#endif // UIP_CONF_IPV6_RPL

#if (UIP_MAX_ROUTES != 0)
    if (uip_ds6_route_num_routes() > 0) {
        uip_ds6_route_t *route;
        cli_uart_output_format("Routing entries (%u in total):\r\n", uip_ds6_route_num_routes());
        route = uip_ds6_route_head();
        while (route != NULL) {
            cli_uart_output_format("-- ");
            cli_output_6addr(&route->ipaddr);
            cli_uart_output_format(" via ");
            cli_output_6addr(uip_ds6_route_nexthop(route));
            if ((unsigned long)route->state.lifetime != 0xFFFFFFFF) {
                cli_uart_output_format(" (lifetime: %lu seconds)\r\n", (unsigned long)route->state.lifetime);
            } else {
                cli_uart_output_format(" (lifetime: infinite)\r\n");
            }
            route = uip_ds6_route_next(route);
        }
    } else {
        cli_uart_output_format("No routing entries\r\n");
    }
#endif // (UIP_MAX_ROUTES != 0)
}

static void command_ping(int argc, char *argv[])
{
    if (argc == 0) {
        cli_uart_output_format("Destination IPv6 address is not specified\r\n");
        return;
    }

    if (uiplib_ipaddrconv(argv[0], &ping_remote_addr) == 0) {
        cli_uart_output_format("Invalid IPv6 address: %s\r\n", argv[0]);
        return;
    }

    curr_ping_times = 1;
    curr_ping_counter = 0;

    if (argc == 2) {
        // user specified number of ping times
        cli_parse_unsigned_long(argv[1], &curr_ping_times);
    }

    cli_uart_output_format("Pinging ");
    cli_output_6addr(&ping_remote_addr);
    cli_uart_output_format(" (%lu times)\r\n", curr_ping_times);

    process_post(&cli_ping_process, PROCESS_EVENT_INIT, NULL);
}

PROCESS_THREAD(cli_ping_process, ev, data)
{
    static struct etimer ping_timeout_timer;

    PROCESS_BEGIN();

    while (1) {
        PROCESS_WAIT_EVENT_UNTIL((ev == PROCESS_EVENT_INIT) ||
                                 (ev == PROCESS_EVENT_POLL) ||
                                 (ev == PROCESS_EVENT_TIMER));

        if (ev == PROCESS_EVENT_INIT) {
            etimer_set(&ping_timeout_timer, PING_TIMEOUT);
            uip_icmp6_send(&ping_remote_addr, ICMP6_ECHO_REQUEST, 0, 4);
        }

        if (ev == PROCESS_EVENT_POLL && !etimer_expired(&ping_timeout_timer)) {
            etimer_stop(&ping_timeout_timer);
            cli_uart_output_format("Received ping reply from ");
            cli_output_6addr(&ping_remote_addr);
            cli_uart_output_format(", len %u, ttl %u, delay %lu ms\r\n",
                    curr_ping_datalen, curr_ping_ttl,
                    (1000*(clock_time() - ping_timeout_timer.timer.start))/CLOCK_SECOND);
            curr_ping_counter++;
            if (curr_ping_times != curr_ping_counter) {
                etimer_set(&ping_timeout_timer, PING_TIMEOUT);
                uip_icmp6_send(&ping_remote_addr, ICMP6_ECHO_REQUEST, 0, 4);
            }
        }

        if (ev == PROCESS_EVENT_TIMER && etimer_expired(&ping_timeout_timer)) {
            cli_uart_output_format("Timeout\r\n");
        }
    }

    PROCESS_END();
}

static void command_exit(int argc, char *argv[])
{
    exit(EXIT_SUCCESS);
}

#if APP_CONF_WITH_COAP
static void command_coap_server_start(int argc, char *argv[])
{
    // set this node as coap-server
    cli_uart_output_format("Starting coap server\r\n");
    process_start(&ns_coap_server, NULL);
}

static void command_coap_client_ep(int argc, char *argv[])
{
    // set server end-point for this client-node
    if (argc == 0) {
        cli_uart_output_format("Need IPv6 coap server end-point addr\r\n");
        return;
    }
    // make sure it's valid IPv6 address
    if (uiplib_ipaddrconv(argv[0], &coap_ep_ipaddr) == 0) {
        cli_uart_output_format("Invalid IPv6 addr: %s\r\n", argv[0]);
        return;
    }

    strcat(server_ep_buf, "coap://[");
    strcat(server_ep_buf, argv[0]);
    strcat(server_ep_buf, "]");

    cli_uart_output_format("Set coap client server end-point: %s\r\n", &server_ep_buf);

    coap_endpoint_parse((char *)&server_ep_buf,
                        ns_strlen((char *)&server_ep_buf),
                        &server_ep);
}

static void coap_client_message_handler(coap_message_t *response)
{
    const uint8_t *msg;
    int len = coap_get_payload(response, &msg);
    printf("|%.*s", len, (char *)msg);
}

static void coap_obs_notif_callback(coap_observee_t *obs, void *notification,
                                    coap_notification_flag_t flag)
{
    int len = 0;
    const uint8_t *payload = NULL;

    cli_uart_output_format("Notification handler\r\n");
    cli_uart_output_format("Observe URI: %s\r\n", obs->url);
    if (notification) {
        len = coap_get_payload(notification, &payload);
    }
    switch (flag) {
    case NOTIFICATION_OK:
        cli_uart_output_format("NOTIFICATION_OK: %*s\r\n", len, (char *)payload);
        break;
    case OBSERVE_OK: // server accepted observation request
        cli_uart_output_format("OBSERVE_OK: %*s\r\n", len, (char *)payload);
        break;
    case OBSERVE_NOT_SUPPORTED:
        cli_uart_output_format("OBSERVE_NOT_SUPPORTED: %*s\r\n", len, (char *)payload);
        break;
    case ERROR_RESPONSE_CODE:
        cli_uart_output_format("ERROR_RESPONSE_CODE: %*s\r\n", len, (char *)payload);
        obs = NULL;
        break;
    case NO_REPLY_FROM_SERVER:
        cli_uart_output_format("NO_REPLY_FROM_SERVER: "
                               "removing observe registration with token %x%x\r\n",
                               obs->token[0], obs->token[1]);
        obs = NULL;
        break;
    }
}

static void parse_uri_path(char *uri_buf, char *arg)
{
    char *uri_query = arg;
    int i;
    for (i = 0; i < ns_strlen(arg); i++) {
        if (*uri_query == '?') {
            break;
        }
        uri_query++;
    }
    ns_strncpy(uri_buf, arg, i + 1);
    if (*uri_query == '?') {
        // got a query, set to coap header
        coap_set_header_uri_query(request, uri_query);
    }
}

static void command_coap_client_obs(int argc, char *argv[])
{
    if (argc == 0) {
        cli_uart_output_format("Invalid uri-path\r\n");
        return;
    }
    // make sure there is no query on uri-path
    char *uri = argv[0];
    for (int i = 0; i < ns_strlen(argv[0]); i++) {
        if (*uri == '?') {
            cli_uart_output_format("Can't observe uri-path with query\r\n");
            return;
        }
        uri++;
    }
    cli_uart_output_format("Coap observe: %s\r\n", argv[0]);
    cli_uart_output_format("Starting observation\r\n");
    obs = coap_obs_request_registration(&server_ep, argv[0], coap_obs_notif_callback, NULL);
}

static void command_coap_client_get(int argc, char *argv[])
{
    if (argc == 0) {
        cli_uart_output_format("Invalid uri-path\r\n");
        return;
    }
    cli_uart_output_format("Coap request: %s", argv[0]);
    coap_init_message(request, COAP_TYPE_CON, COAP_GET, 0);
    char uri_path[64];
    parse_uri_path((char *)&uri_path, argv[0]);
    coap_set_header_uri_path(request, (char *)&uri_path);
    cli_uart_output_format("\r\n");
    if (!process_is_running(&ns_coap_client)) {
        process_start(&ns_coap_client, NULL);
    }
    process_poll(&ns_coap_client);
}

PROCESS_THREAD(ns_coap_client, ev, data)
{
    PROCESS_BEGIN();

    while (1) {
        PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_POLL);
        COAP_BLOCKING_REQUEST(&server_ep, request, coap_client_message_handler);
        ns_log("\r\n-- done --\r\n");
    }

    PROCESS_END();
}
#endif // #if APP_CONF_WITH_COAP

// cli helper function
void cli_process_line(char *buf, uint16_t buf_len)
{
    char *argv[const_max_args] = {NULL};
    char *cmd;
    uint8_t argc = 0, i = 0;

    if (buf == NULL) {
        return;
    }

    for (; *buf == ' '; buf++, buf_len--)
        ;

    for (cmd = buf + 1; (cmd < buf + buf_len) && (cmd != NULL); ++cmd) {
        if (argc > const_max_args) {
            return; // to many args
        }
        if (*cmd == ' ' || *cmd == '\r' || *cmd == '\n') {
            *cmd = '\0';
        }
        if (*(cmd - 1) == '\0' && *cmd != ' ') {
            argv[argc++] = cmd;
        }
    }

    cmd = buf;

    for (i = 0; i < sizeof(s_commands) / sizeof(s_commands[0]); i++) {
        if (ns_strcmp(cmd, s_commands[i].name) == 0) {
            (*s_commands[i].command)(argc, argv);
            break;
        }
    }

    if (i == (sizeof(s_commands) / sizeof(s_commands[0]))) {
        cli_uart_output_format("unknown command: %s\r\n", cmd);
    }
}

int cli_parse_long(char *str, long *l)
{
    char *endptr;
    *l = strtol(str, &endptr, 0);
    return (*endptr == '\0') ? 0 : -1;
}

int cli_parse_unsigned_long(char *str, unsigned long *ul)
{
    char *endptr;
    *ul = strtoul(str, &endptr, 0);
    return (*endptr == '\0') ? 0 : -1;
}

void cli_output_bytes(const uint8_t *bytes, uint8_t len)
{
    for (int i = 0; i < len; i++) {
        cli_uart_output_format("%02x", bytes[i]);
    }
}

void cli_commands_init(void)
{
    process_start(&cli_ping_process, NULL);
    // set up ping reply callback
    uip_icmp6_echo_reply_callback_add(&echo_reply_notification,
                                      echo_reply_handler);
}

static void cli_output_6addr(const uip_ipaddr_t *ipaddr)
{
    char buf[UIPLIB_IPV6_MAX_STR_LEN];
    uiplib_ipaddr_snprint(buf, sizeof(buf), ipaddr);
    cli_uart_output_format("%s", buf);
}

static void cli_output_lladdr(const linkaddr_t *lladdr)
{
    if (lladdr == NULL) {
        cli_uart_output_format("(NULL LL addr)");
        return;
    } else {
        unsigned int i;
        for (i = 0; i < LINKADDR_SIZE; i++) {
            if (i > 0 && i % 2 == 0) {
                cli_uart_output_format(".");
            }
            cli_uart_output_format("%02x", lladdr->u8[i]);
        }
    }
}

static const char * ds6_nbr_state_to_str(uint8_t state)
{
    switch (state) {
    case NBR_INCOMPLETE:
        return "Incomplete";
    case NBR_REACHABLE:
        return "Reachable";
    case NBR_STALE:
        return "Stale";
    case NBR_DELAY:
        return "Delay";
    case NBR_PROBE:
        return "Probe";
    default:
        return "Unknown";
    }
}

#if ROUTING_CONF_RPL_LITE
static const char * rpl_state_to_str(enum rpl_dag_state state)
{
    switch (state) {
    case DAG_INITIALIZED:
        return "Initialized";
    case DAG_JOINED:
        return "Joined";
    case DAG_REACHABLE:
        return "Reachable";
    case DAG_POISONING:
        return "Poisoning";
    default:
        return "Unknown";
    }
}

static const char *rpl_mop_to_str(int mop)
{
    switch (mop) {
    case RPL_MOP_NO_DOWNWARD_ROUTES:
        return "No downward routes";
    case RPL_MOP_NON_STORING:
        return "Non-storing";
    case RPL_MOP_STORING_NO_MULTICAST:
        return "Storing";
    case RPL_MOP_STORING_MULTICAST:
        return "Storing+multicast";
    default:
        return "Unknown";
    }
}

static const char *rpl_ocp_to_str(int ocp)
{
    switch (ocp) {
    case RPL_OCP_OF0:
        return "OF0";
    case RPL_OCP_MRHOF:
        return "MRHOF";
    default:
        return "Unknown";
    }
}
#endif // ROUTING_CONF_RPL_LITE

static void echo_reply_handler(uip_ipaddr_t *source, uint8_t ttl, uint8_t *data, uint16_t datalen)
{
    curr_ping_ttl = ttl;
    curr_ping_datalen = datalen;
    process_poll(&cli_ping_process);
}
