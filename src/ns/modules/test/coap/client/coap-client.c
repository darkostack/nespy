#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ns/contiki.h"
#include "ns/contiki-net.h"
#include "ns/net/app-layer/coap/coap-engine.h"
#include "ns/net/app-layer/coap/coap-blocking-api.h"
#include "ns/modules/nstd.h"

// Log configuration
#include "ns/net/app-layer/coap/coap-log.h"
#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_APP

// FIXME: This is hardcoded address to leaf node 1
#define SERVER_EP "coap://[fe80::200:0:0:1]"

#define TOGGLE_INTERVAL 3

static struct etimer et;

PROCESS(ns_coap_client, "Nespy coap client");

void client_chunk_handler(coap_message_t *response)
{
    const uint8_t *chunk;
    int len = coap_get_payload(response, &chunk);
    printf("|%.*s", len, (char *)chunk);
}

PROCESS_THREAD(ns_coap_client, ev, data)
{
    static coap_endpoint_t server_ep;
    PROCESS_BEGIN();

    static coap_message_t request[1];
    coap_endpoint_parse(SERVER_EP, ns_strlen(SERVER_EP), &server_ep);
    etimer_set(&et, TOGGLE_INTERVAL * CLOCK_SECOND);

    while (1) {
        PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_TIMER);
        if (etimer_expired(&et)) {
            ns_log("-- toggle timer expired --\r\n");   

            // prepare request, TID is set by COAP_BLOCKING_REQUEST()
            coap_init_message(request, COAP_TYPE_CON, COAP_GET, 0);
            coap_set_header_uri_path(request, "test/hello");

            LOG_INFO_COAP_EP(&server_ep);
            LOG_INFO_("\r\n");

            COAP_BLOCKING_REQUEST(&server_ep, request, client_chunk_handler);
            ns_log("\r\n-- done --\r\n");

            etimer_reset(&et);
        }
    }

    PROCESS_END();
}
