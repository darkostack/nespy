#include "ns/include/error.h"
#include "ns/include/nstd.h"
#include "ns/sys/core/common/instance.h"
#include <string.h>
#include <stdio.h>

ns_error_t
test_message_priority_queue(void)
{
    ns_error_t error = NS_ERROR_NONE;
    instance_t *inst = instance_get();
    uint8_t num_of_test_messages = 5;
    priority_queue_t priority_queue;
    message_t msg[num_of_test_messages];
    uint16_t msg_count, buffer_count;

    printf("---------------- TEST MESSAGE PRIORITY QUEUE\r\n");

    message_priority_queue_make_new(&priority_queue);

    for (int i = 0; i < num_of_test_messages; i++) {
        msg[i] = message_new(0, 0, 0);
        if (msg[i] == NULL) {
            printf("failed to create the message!\r\n");
            EXIT_NOW();
        }
    }

    error = message_priority_queue_enqueue(&priority_queue, msg[0]);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);

    message_priority_queue_get_info(&priority_queue, &msg_count, &buffer_count);
    printf("priority_queue msg_count: %u\r\n", msg_count);

    message_priority_queue_get_info(&inst->message_pool.all_queue, &msg_count, &buffer_count);
    printf("message pool all queue msg_count: %u\r\n", msg_count);

exit:
    if (error != NS_ERROR_NONE) {
        printf("message test priority queue FAILED\r\n");
    } else {
        printf("message test priority queue SUCCESS\r\n");
    }

    return error;
}
