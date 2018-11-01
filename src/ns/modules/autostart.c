#include "ns/contiki.h"
#include "ns/modules/nstd.h"
#include <stdio.h>

PROCESS(modules_autostart, "modules autostart process");
AUTOSTART_PROCESSES(&modules_autostart);

PROCESS_THREAD(modules_autostart, ev, data)
{
    PROCESS_BEGIN();
    while (1) {
        PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_POLL);
        ns_log("ns: modules autostart: event poll! ");
        if (data != NULL) {
            ns_log("data: %d\n", *(int *)data); 
        }  else {
            ns_log("data: NULL\n");
        }
    }

    PROCESS_END();
}
