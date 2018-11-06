#include <stdlib.h>
#include <string.h>
#include "ns/net/app-layer/coap/coap-engine.h"

static void res_get_handler(coap_message_t *request,
                            coap_message_t *response,
                            uint8_t *buffer,
                            uint16_t preferred_size,
                            int32_t *offset);

RESOURCE(res_hello,
         "title=\"Hello world: ?len=0..\";rt=\"Text\"",
         res_get_handler,
         NULL,
         NULL,
         NULL);

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
    coap_set_header_etag(response, (uint8_t *)&length, 1);
    coap_set_payload(response, buffer, length);
}
