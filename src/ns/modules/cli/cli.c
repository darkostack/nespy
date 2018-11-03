#include "ns/contiki.h"
#include "ns/contiki-net.h"
#include "ns/sys/node-id.h"

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

#if defined(UNIX)
extern uint16_t unix_radio_get_port(void);
#endif

// commands
static void command_help(int argc, char *argv[]);
static void command_ps(int argc, char *argv[]);
static void command_version(int argc, char *argv[]);
static void command_send(int argc, char *argv[]);
static void command_rpl_status(int argc, char *argv[]);
static void command_rpl_nbr(int argc, char *argv[]);

// helper function
static void cli_output_6addr(const uip_ipaddr_t *ipaddr);

static const ns_cli_cmd_t s_commands[] = {
    { "help", &command_help },
    { "ps", &command_ps },
    { "version", &command_version },
    { "send", &command_send },
    { "rpl-status", &command_rpl_status },
    { "rpl-nbr", &command_rpl_nbr },
};

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
#endif

static void command_help(int argc, char *argv[])
{
    for (unsigned int i = 0; i < (sizeof(s_commands) / sizeof(s_commands[0])); i++) {
        cli_uart_output_format("%s\r\n", s_commands[i].name);
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

static void command_send(int argc, char *argv[])
{
    uint8_t buf[10];
    for (int i = 0; i < sizeof(buf); i++) {
        buf[i] = i;
    }
    NETSTACK_RADIO.send((uint8_t *)&buf, sizeof(buf));
}

static void command_rpl_status(int argc, char *argv[])
{
#if ROUTING_CONF_RPL_LITE
    cli_uart_output_format("RPL status:\r\n");
    if (!curr_instance.used) {
        cli_uart_output_format("-- Instance: None\r\n");
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
            cli_uart_output_format("None\r\n");
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
#endif
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

static void cli_output_6addr(const uip_ipaddr_t *ipaddr)
{
    char buf[UIPLIB_IPV6_MAX_STR_LEN];
    uiplib_ipaddr_snprint(buf, sizeof(buf), ipaddr);
    cli_uart_output_format("%s", buf);
}
