#ifndef NS_CLI_H_
#define NS_CLI_H_

#include <stdarg.h>
#include <stdint.h>

enum {
    CLI_MAX_ARGS = 32,
};

typedef struct cli_cmd_t {
    const char *name;
    const char *desc;
    void (*command)(int argc, char *argv[]);
} cli_cmd_t;

void cli_process_line(char *buf, uint16_t buf_len);
int cli_parse_long(char *str, long *l);
int cli_parse_unsigned_long(char *str, unsigned long *ul);
void cli_output_bytes(const uint8_t *bytes, uint8_t len);
void cli_commands_init(void);

#endif // NS_CLI_H_
