#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ns/net/app-layer/coap/coap-engine.h"

// Log configuration
#include "ns/net/app-layer/coap/coap-log.h"
#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_APP

extern coap_resource_t res_hello;

PROCESS(ns_coap_server, "Nespy coap server");

PROCESS_THREAD(ns_coap_server, ev, data)
{
    PROCESS_BEGIN();
    PROCESS_PAUSE();

    LOG_INFO("Starting Nespy coap server\r\n");

    // bind the resources to their Uri-Path
    coap_activate_resource(&res_hello, "test/hello");

    while (1) {
        PROCESS_WAIT_EVENT();
    }

    PROCESS_END();
}
