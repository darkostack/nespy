#include "ns/contiki.h"
#include <stdio.h>

PROCESS(modules_autostart, "modules autostart process");
AUTOSTART_PROCESSES(&modules_autostart);

PROCESS_THREAD(modules_autostart, ev, data)
{
    PROCESS_BEGIN();
    while (1) {
        PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_POLL);
        printf("ns: modules autostart: event poll! ");
        if (data != NULL) {
            printf("data: %d\n", *(int *)data); 
        }  else {
            printf("data: NULL\n");
        }
    }

    PROCESS_END();
}
