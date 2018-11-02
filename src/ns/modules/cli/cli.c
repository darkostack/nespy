#include "ns/contiki.h"
#include "ns/contiki-net.h"
#include "ns/net/netstack.h"
#include "ns/sys/node-id.h"
#include "ns/net/mac/tsch/tsch.h"
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
static void command_netinfo(int argc, char *argv[]);

static const ns_cli_cmd_t s_commands[] = {
    { "help", &command_help },
    { "ps", &command_ps },
    { "version", &command_version },
    { "send", &command_send },
    { "netinfo", &command_netinfo },
};

static void command_help(int argc, char *argv[])
{
    for (unsigned int i = 0; i < (sizeof(s_commands) / sizeof(s_commands[0])); i++) {
        cli_uart_output_format("%s\r\n", s_commands[i].name);
    }
}

static void command_ps(int argc, char *argv[])
{
    struct process *p;
    for (p = process_list; p != NULL; p = p->next) {
        cli_uart_output_format("%s\r\n", p->name);
    }
}

static void command_version(int argc, char *argv[])
{
    cli_uart_output_format("Nespy v%s %s : build-date %s\r\n",
                           MICROPY_VERSION_STRING,
                           MICROPY_GIT_TAG,
                           MICROPY_BUILD_DATE);
}

static void command_send(int argc, char *argv[])
{
    uint8_t buf[10];
    for (int i = 0; i < sizeof(buf); i++) {
        buf[i] = i;
    }
    NETSTACK_RADIO.send((uint8_t *)&buf, sizeof(buf));
}

static void command_netinfo(int argc, char *argv[])
{
    cli_uart_output_format("Routing: %s\r\n", NETSTACK_ROUTING.name);
    cli_uart_output_format("Net: %s\r\n", NETSTACK_NETWORK.name);
    cli_uart_output_format("MAC: %s\r\n", NETSTACK_MAC.name);
    cli_uart_output_format("802.15.4 PANID: 0x%04x\r\n", IEEE802154_PANID);
#if defined(UNIX)
    cli_uart_output_format("Radio PORT: %u\r\n", unix_radio_get_port());
#endif
#if MAC_CONF_WITH_TSCH
    cli_uart_output_format("802.15.4 TSCH default hopping sequence length: %u\r\n",
              (unsigned)sizeof(TSCH_DEFAULT_HOPPING_SEQUENCE));
#else
    cli_uart_output_format("802.15.4 Default channel: %u\r\n", IEEE802154_DEFAULT_CHANNEL);
#endif
    cli_uart_output_format("Node ID: %u\r\n", node_id);
    cli_uart_output_format("Link-layer address: ");

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
    cli_uart_output_format("Tentative link-local IPv6 address: %s\r\n", buf);
#endif
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

