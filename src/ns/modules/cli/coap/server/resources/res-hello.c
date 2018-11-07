#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "ns/net/app-layer/coap/coap-engine.h"
#include "ns/net/app-layer/coap/coap.h"

static void res_get_handler(coap_message_t *request,
                            coap_message_t *response,
                            uint8_t *buffer,
                            uint16_t preferred_size,
                            int32_t *offset);

static void res_periodic_handler(void);

PERIODIC_RESOURCE(res_hello,
                  "title=\"Hello world\";obs",
                  res_get_handler,
                  NULL,
                  NULL,
                  NULL,
                  1000, // milliseconds resolutions
                  res_periodic_handler);

static void res_get_handler(coap_message_t *request,
                            coap_message_t *response,
                            uint8_t *buffer,
                            uint16_t preferred_size,
                            int32_t *offset)
{
    const char *len = NULL;
    char const *const message = "Hello World! ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxy";
    int length = 12;

    if (coap_get_query_variable(request, "len", &len)) {
        length = atoi(len);
        if (length < 0) {
            length = 0;
        }
        if (length > REST_MAX_CHUNK_SIZE) {
            length = REST_MAX_CHUNK_SIZE;
        }
        memcpy(buffer, message, length);
    } else {
        memcpy(buffer, message, length);
    }

    coap_set_header_content_format(response, TEXT_PLAIN);
    coap_set_header_max_age(response, res_hello.periodic->period / CLOCK_SECOND);
    coap_set_payload(response, buffer, length);
}

static void res_periodic_handler(void)
{
    coap_notify_observers(&res_hello);
}
