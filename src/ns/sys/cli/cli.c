#include "ns/sys/cli/cli.h"
#include "ns/sys/cli/cli-uart.h"
#include "ns/include/nstd.h"
#include <string.h>

// commands
static void command_help(int argc, char *argv[]);

static const cli_cmd_t commands[] = {
    { "help", "shows list of available commands", &command_help },
};

static void command_help(int argc, char *argv[])
{
    cli_uart_output_format("%-20s %s\r\n", "Command", "Description");
    cli_uart_output_format("---------------------------------------\r\n");
    for (unsigned int i = 0; i < (sizeof(commands) / sizeof(commands[0])); i++) {
        cli_uart_output_format("%-20s %s\r\n", commands[i].name, commands[i].desc);
    }
}

// cli helper function
void cli_process_line(char *buf, uint16_t buf_len)
{
    char *argv[CLI_MAX_ARGS] = {NULL};
    char *cmd;
    uint8_t argc = 0, i = 0;

    if (buf == NULL) {
        return;
    }

    for (; *buf == ' '; buf++, buf_len--)
        ;

    for (cmd = buf + 1; (cmd < buf + buf_len) && (cmd != NULL); ++cmd) {
        if (argc > CLI_MAX_ARGS) {
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

    for (i = 0; i < sizeof(commands) / sizeof(commands[0]); i++) {
        if (ns_strcmp(cmd, commands[i].name) == 0) {
            (*commands[i].command)(argc, argv);
            break;
        }
    }

    if (i == (sizeof(commands) / sizeof(commands[0]))) {
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
    // Note: place your command init here
}
