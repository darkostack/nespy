#include "ns/include/error.h"
#include "ns/include/nstd.h"
#include "ns/sys/core/common/instance.h"
#include <string.h>
#include <stdio.h>

// MESSAGE TEST FUNCTIONS -----------------------------------------------------
// --- private functions declarations
static ns_error_t
verify_message_queue_content(message_queue_t *queue, int expected_length, ...);

// --- message test functions
void
test_message_write_read(void)
{
    instance_t *inst = instance_get();

    printf("-------------------- TEST MESSAGE WRITE READ\r\n");

    uint8_t write_buffer[1024];
    uint8_t read_buffer[1024];

    extern uint32_t ns_plat_random_get(void);

    for (unsigned i = 0; i < sizeof(write_buffer); i++) {
        write_buffer[i] = (uint8_t)ns_plat_random_get();
    }

    message_t message = message_new(0, 0, 0);

    printf("num of free buffers: %u\r\n", inst->message_pool.num_free_buffers);

    message_set_length(message, sizeof(write_buffer));
    message_write(message, 0, write_buffer, sizeof(write_buffer));
    message_read(message, 0, read_buffer, sizeof(read_buffer));

    if (memcmp(write_buffer, read_buffer, sizeof(write_buffer)) == 0) {
        printf("message write and read test SUCCESS\r\n");
    } else {
        printf("message write and read test FAILED\r\n");
    }

    printf("message get length: %u\r\n", message_get_length(message));

    message_free(message);

    printf("freed message, num of free buffers now: %u\r\n", inst->message_pool.num_free_buffers);
}

void
test_message_queue(void)
{
    uint8_t num_of_test_messages = 5;
    message_queue_t message_queue;
    message_t msg[num_of_test_messages];
    ns_error_t error = NS_ERROR_NONE;
    uint16_t msg_count, buffer_count;

    printf("------------------------- TEST MESSAGE QUEUE\r\n");

    // Note: this is a must otherwise it will cause hard-fault
    message_queue_make_new(&message_queue);

    for (int i = 0; i < num_of_test_messages; i++) {
        msg[i] = message_new(0, 0, 0);
        if (msg[i] == NULL) {
            printf("failed to create the message!\r\n");
            EXIT_NOW();
        }
    }

    error = verify_message_queue_content(&message_queue, 0);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);

    // enqueue 1 message and remove it
    error = message_queue_enqueue(&message_queue, msg[0], MSG_QUEUE_POS_TAIL);
    error = verify_message_queue_content(&message_queue, 1, msg[0]);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);

    error = message_queue_dequeue(&message_queue, msg[0]);
    error = verify_message_queue_content(&message_queue, 0);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);

    // enqueue 1 message at head and remove it
    error = message_queue_enqueue(&message_queue, msg[0], MSG_QUEUE_POS_HEAD);
    error = verify_message_queue_content(&message_queue, 1, msg[0]);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);

    error = message_queue_dequeue(&message_queue, msg[0]);
    error = verify_message_queue_content(&message_queue, 0);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);

    // enqueue 5 messages
    error = message_queue_enqueue(&message_queue, msg[0], MSG_QUEUE_POS_TAIL);
    error = verify_message_queue_content(&message_queue, 1, msg[0]);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);

    error = message_queue_enqueue(&message_queue, msg[1], MSG_QUEUE_POS_TAIL);
    error = verify_message_queue_content(&message_queue, 2, msg[0], msg[1]);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);

    error = message_queue_enqueue(&message_queue, msg[2], MSG_QUEUE_POS_TAIL);
    error = verify_message_queue_content(&message_queue, 3, msg[0], msg[1], msg[2]);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);

    error = message_queue_enqueue(&message_queue, msg[3], MSG_QUEUE_POS_TAIL);
    error = verify_message_queue_content(&message_queue, 4, msg[0], msg[1], msg[2], msg[3]);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);

    error = message_queue_enqueue(&message_queue, msg[4], MSG_QUEUE_POS_TAIL);
    error = verify_message_queue_content(&message_queue, 5, msg[0], msg[1], msg[2], msg[3], msg[4]);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);

    // check get info
    message_queue_get_info(&message_queue, &msg_count, &buffer_count);
    if (msg_count != 5 ) {
        printf("ERROR: message count: %u, expect 5\r\n", msg_count);
        EXIT_NOW(error = NS_ERROR_FAILED);
    }

    // remove message in head
    error = message_queue_dequeue(&message_queue, msg[0]);
    error = verify_message_queue_content(&message_queue, 4, msg[1], msg[2], msg[3], msg[4]);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);

    // remove message in middle
    error = message_queue_dequeue(&message_queue, msg[3]);
    error = verify_message_queue_content(&message_queue, 3, msg[1], msg[2], msg[4]);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);

    // remove message from tail
    error = message_queue_dequeue(&message_queue, msg[4]);
    error = verify_message_queue_content(&message_queue, 2, msg[1], msg[2]);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);

    // add after remove
    error = message_queue_enqueue(&message_queue, msg[0], MSG_QUEUE_POS_TAIL);
    error = verify_message_queue_content(&message_queue, 3, msg[1], msg[2], msg[0]);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);

    error = message_queue_enqueue(&message_queue, msg[3], MSG_QUEUE_POS_TAIL);
    error = verify_message_queue_content(&message_queue, 4, msg[1], msg[2], msg[0], msg[3]);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);

    // remove from middle
    error = message_queue_dequeue(&message_queue, msg[2]);
    error = verify_message_queue_content(&message_queue, 3, msg[1], msg[0], msg[3]);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);

    // add to head
    error = message_queue_enqueue(&message_queue, msg[2], MSG_QUEUE_POS_HEAD);
    error = verify_message_queue_content(&message_queue, 4, msg[2], msg[1], msg[0], msg[3]);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);

    // remove from head
    error = message_queue_dequeue(&message_queue, msg[2]);
    error = verify_message_queue_content(&message_queue, 3, msg[1], msg[0], msg[3]);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);

    // remove from head
    error = message_queue_dequeue(&message_queue, msg[1]);
    error = verify_message_queue_content(&message_queue, 2, msg[0], msg[3]);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);

    // add to head
    error = message_queue_enqueue(&message_queue, msg[1], MSG_QUEUE_POS_HEAD);
    error = verify_message_queue_content(&message_queue, 3, msg[1], msg[0], msg[3]);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);

    // add to tail
    error = message_queue_enqueue(&message_queue, msg[2], MSG_QUEUE_POS_TAIL);
    error = verify_message_queue_content(&message_queue, 4, msg[1], msg[0], msg[3], msg[2]);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);

    // remove all messages
    error = message_queue_dequeue(&message_queue, msg[3]);
    error = verify_message_queue_content(&message_queue, 3, msg[1], msg[0], msg[2]);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);

    error = message_queue_dequeue(&message_queue, msg[1]);
    error = verify_message_queue_content(&message_queue, 2, msg[0], msg[2]);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);

    error = message_queue_dequeue(&message_queue, msg[2]);
    error = verify_message_queue_content(&message_queue, 1, msg[0]);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);

    error = message_queue_dequeue(&message_queue, msg[0]);
    error = verify_message_queue_content(&message_queue, 0);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);

    // check the failure cases
    error = message_queue_enqueue(&message_queue, msg[0], MSG_QUEUE_POS_TAIL);
    error = verify_message_queue_content(&message_queue, 1, msg[0]);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);
    // enqueue already queued message
    error = message_queue_enqueue(&message_queue, msg[0], MSG_QUEUE_POS_TAIL);
    VERIFY_OR_EXIT(error == NS_ERROR_ALREADY);
    // dequeue not queued message
    error = message_queue_dequeue(&message_queue, msg[1]);
    VERIFY_OR_EXIT(error == NS_ERROR_NOT_FOUND);

    error = NS_ERROR_NONE;

exit:
    if (error != NS_ERROR_NONE) {
        printf("message queue test FAILED\r\n");
    } else {
        printf("message queue test SUCCESS\r\n");
    }
    return;
}

// --- private functions
static ns_error_t
verify_message_queue_content(message_queue_t *queue, int expected_length, ...)
{
    ns_error_t error = NS_ERROR_NONE;
    va_list args;
    message_t message;
    message_t msg_arg;

    va_start(args, expected_length);

    if (expected_length == 0) {
        message = message_queue_get_head(queue);
        if (message != NULL) {
            printf("message queue is not empty when expected length is zero.\r\n");
            EXIT_NOW(error = NS_ERROR_FAILED);
        }
    } else {
        for (message = message_queue_get_head(queue); message != NULL; message = message_get_next(message)) {
            if (expected_length == 0) {
                printf("message queue contains more entries than expected.\r\n");
                EXIT_NOW(error = NS_ERROR_FAILED);
            }

            msg_arg = va_arg(args, message_t);

            if (msg_arg != message) {
                printf("message queue content does not match what is expected.\r\n");
                EXIT_NOW(error = NS_ERROR_FAILED);
            }

            expected_length--;
        }

        if (expected_length != 0) {
            printf("message queue contains less entries than expected\r\n");
            EXIT_NOW(error = NS_ERROR_FAILED);
        }
    }

exit:
    va_end(args);
    return error;
}
// END OF MESSAGE TEST FUNCTIONS -----------------------------------------------
