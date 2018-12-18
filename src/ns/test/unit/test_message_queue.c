#include "ns/include/error.h"
#include "ns/sys/core/common/instance.h"
#include "ns/test/unit/test_util.h"
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

// this function verifies the content of the message queue to match the passed in messages
static ns_error_t
verify_message_queue_content(message_queue_t *queue, int expected_length, ...);

ns_error_t
test_message_queue(void *instance)
{
    message_pool_t *message_pool = &((instance_t *)instance)->message_pool;

    uint8_t num_of_test_messages = 5;
    message_queue_t message_queue;
    message_t msg[num_of_test_messages];
    ns_error_t error = NS_ERROR_NONE;
    uint16_t msg_count, buffer_count;

    printf("------------------------- TEST MESSAGE QUEUE\r\n");

    // Note: this is a must otherwise it will cause hard-fault
    message_queue_ctor(&message_queue);

    for (int i = 0; i < num_of_test_messages; i++) {
        msg[i] = message_new(0, 0, 0);
        TEST_VERIFY_OR_EXIT(msg[i] != NULL, "message new failed.\r\n");
    }

    error = verify_message_queue_content(&message_queue, 0);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);

    // enqueue 1 message and remove it
    TEST_VERIFY_OR_EXIT(message_queue_enqueue(&message_queue, msg[0], MSG_QUEUE_POS_TAIL) == NS_ERROR_NONE,
                        "message queue enqueue failed.\r\n");
    error = verify_message_queue_content(&message_queue, 1, msg[0]);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);

    TEST_VERIFY_OR_EXIT(message_queue_dequeue(&message_queue, msg[0]) == NS_ERROR_NONE,
                        "message queue dequeue failed.\r\n");
    error = verify_message_queue_content(&message_queue, 0);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);

    // enqueue 1 message at head and remove it
    TEST_VERIFY_OR_EXIT(message_queue_enqueue(&message_queue, msg[0], MSG_QUEUE_POS_HEAD) == NS_ERROR_NONE,
                        "message queue enqueue failed.\r\n");
    error = verify_message_queue_content(&message_queue, 1, msg[0]);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);

    TEST_VERIFY_OR_EXIT(message_queue_dequeue(&message_queue, msg[0]) == NS_ERROR_NONE,
                        "message queue dequeue failed.\r\n");
    error = verify_message_queue_content(&message_queue, 0);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);

    // enqueue 5 messages
    TEST_VERIFY_OR_EXIT(message_queue_enqueue(&message_queue, msg[0], MSG_QUEUE_POS_TAIL) == NS_ERROR_NONE,
                        "message queue enqueue failed.\r\n");
    error = verify_message_queue_content(&message_queue, 1, msg[0]);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);

    TEST_VERIFY_OR_EXIT(message_queue_enqueue(&message_queue, msg[1], MSG_QUEUE_POS_TAIL) == NS_ERROR_NONE,
                        "message queue enqueue failed.\r\n");
    error = verify_message_queue_content(&message_queue, 2, msg[0], msg[1]);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);

    TEST_VERIFY_OR_EXIT(message_queue_enqueue(&message_queue, msg[2], MSG_QUEUE_POS_TAIL) == NS_ERROR_NONE,
                        "message queue enqueue failed.\r\n");
    error = verify_message_queue_content(&message_queue, 3, msg[0], msg[1], msg[2]);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);

    TEST_VERIFY_OR_EXIT(message_queue_enqueue(&message_queue, msg[3], MSG_QUEUE_POS_TAIL) == NS_ERROR_NONE,
                        "message queue enqueue failed.\r\n");
    error = verify_message_queue_content(&message_queue, 4, msg[0], msg[1], msg[2], msg[3]);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);

    TEST_VERIFY_OR_EXIT(message_queue_enqueue(&message_queue, msg[4], MSG_QUEUE_POS_TAIL) == NS_ERROR_NONE,
                        "message queue enqueue failed.\r\n");
    error = verify_message_queue_content(&message_queue, 5, msg[0], msg[1], msg[2], msg[3], msg[4]);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);

    // check get info
    message_queue_get_info(&message_queue, &msg_count, &buffer_count);
    TEST_VERIFY_OR_EXIT(msg_count == 5, "message queue get info is not as expected.\r\n");

    // remove message in head
    TEST_VERIFY_OR_EXIT(message_queue_dequeue(&message_queue, msg[0]) == NS_ERROR_NONE,
                        "message queue dequeue failed.\r\n");
    error = verify_message_queue_content(&message_queue, 4, msg[1], msg[2], msg[3], msg[4]);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);

    // remove message in middle
    TEST_VERIFY_OR_EXIT(message_queue_dequeue(&message_queue, msg[3]) == NS_ERROR_NONE,
                        "message queue dequeue failed.\r\n");
    error = verify_message_queue_content(&message_queue, 3, msg[1], msg[2], msg[4]);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);

    // remove message from tail
    TEST_VERIFY_OR_EXIT(message_queue_dequeue(&message_queue, msg[4]) == NS_ERROR_NONE,
                        "message queue dequeue failed.\r\n");
    error = verify_message_queue_content(&message_queue, 2, msg[1], msg[2]);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);

    // add after remove
    TEST_VERIFY_OR_EXIT(message_queue_enqueue(&message_queue, msg[0], MSG_QUEUE_POS_TAIL) == NS_ERROR_NONE,
                        "message queue enqueue failed.\r\n");
    error = verify_message_queue_content(&message_queue, 3, msg[1], msg[2], msg[0]);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);

    TEST_VERIFY_OR_EXIT(message_queue_enqueue(&message_queue, msg[3], MSG_QUEUE_POS_TAIL) == NS_ERROR_NONE,
                        "message queue enqueue failed.\r\n");
    error = verify_message_queue_content(&message_queue, 4, msg[1], msg[2], msg[0], msg[3]);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);

    // remove from middle
    TEST_VERIFY_OR_EXIT(message_queue_dequeue(&message_queue, msg[2]) == NS_ERROR_NONE,
                        "message queue dequeue failed.\r\n");
    error = verify_message_queue_content(&message_queue, 3, msg[1], msg[0], msg[3]);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);

    // add to head
    TEST_VERIFY_OR_EXIT(message_queue_enqueue(&message_queue, msg[2], MSG_QUEUE_POS_HEAD) == NS_ERROR_NONE,
                        "message queue enqueue failed.\r\n");
    error = verify_message_queue_content(&message_queue, 4, msg[2], msg[1], msg[0], msg[3]);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);

    // remove from head
    TEST_VERIFY_OR_EXIT(message_queue_dequeue(&message_queue, msg[2]) == NS_ERROR_NONE,
                        "message queue dequeue failed.\r\n");
    error = verify_message_queue_content(&message_queue, 3, msg[1], msg[0], msg[3]);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);

    // remove from head
    TEST_VERIFY_OR_EXIT(message_queue_dequeue(&message_queue, msg[1]) == NS_ERROR_NONE,
                        "message queue dequeue failed.\r\n");
    error = verify_message_queue_content(&message_queue, 2, msg[0], msg[3]);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);

    // add to head
    TEST_VERIFY_OR_EXIT(message_queue_enqueue(&message_queue, msg[1], MSG_QUEUE_POS_HEAD) == NS_ERROR_NONE,
                        "message queue enqueue failed.\r\n");
    error = verify_message_queue_content(&message_queue, 3, msg[1], msg[0], msg[3]);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);

    // add to tail
    TEST_VERIFY_OR_EXIT(message_queue_enqueue(&message_queue, msg[2], MSG_QUEUE_POS_TAIL) == NS_ERROR_NONE,
                        "message queue enqueue failed.\r\n");
    error = verify_message_queue_content(&message_queue, 4, msg[1], msg[0], msg[3], msg[2]);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);

    // remove all messages
    TEST_VERIFY_OR_EXIT(message_queue_dequeue(&message_queue, msg[3]) == NS_ERROR_NONE,
                        "message queue dequeue failed.\r\n");
    error = verify_message_queue_content(&message_queue, 3, msg[1], msg[0], msg[2]);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);

    TEST_VERIFY_OR_EXIT(message_queue_dequeue(&message_queue, msg[1]) == NS_ERROR_NONE,
                        "message queue dequeue failed.\r\n");
    error = verify_message_queue_content(&message_queue, 2, msg[0], msg[2]);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);

    TEST_VERIFY_OR_EXIT(message_queue_dequeue(&message_queue, msg[2]) == NS_ERROR_NONE,
                        "message queue dequeue failed.\r\n");
    error = verify_message_queue_content(&message_queue, 1, msg[0]);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);

    TEST_VERIFY_OR_EXIT(message_queue_dequeue(&message_queue, msg[0]) == NS_ERROR_NONE,
                        "message queue dequeue failed.\r\n");
    error = verify_message_queue_content(&message_queue, 0);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);

    // check the failure cases
    TEST_VERIFY_OR_EXIT(message_queue_enqueue(&message_queue, msg[0], MSG_QUEUE_POS_TAIL) == NS_ERROR_NONE,
                        "message queue enqueue failed.\r\n");
    error = verify_message_queue_content(&message_queue, 1, msg[0]);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);
    // enqueue already queued message
    TEST_VERIFY_OR_EXIT(message_queue_enqueue(&message_queue, msg[0], MSG_QUEUE_POS_TAIL) == NS_ERROR_ALREADY,
                        "message queue alread enqueue message did not fail as expected.\r\n");
    // dequeue not queued message
    TEST_VERIFY_OR_EXIT(message_queue_dequeue(&message_queue, msg[1]) == NS_ERROR_NOT_FOUND,
                        "message queue dequeue not queued message did not fail as expected.\r\n");

    // remove all message from queue
    TEST_VERIFY_OR_EXIT(message_queue_dequeue(&message_queue, msg[0]) == NS_ERROR_NONE,
                        "message queue dequeue failed.\r\n");

    message_priority_queue_get_info(&message_pool->all_queue, &msg_count, &buffer_count);
    TEST_VERIFY_OR_EXIT(msg_count == 0, "num of message in all queue not zero as expected.\r\n");

    for (int i = 0; i < num_of_test_messages; i++) {
        message_free(msg[i]);
    }

    TEST_VERIFY_OR_EXIT(message_pool->num_free_buffers == MSG_NUM_BUFFERS,
                        "num of free buffers did not match the value expected.\r\n");

exit:
    if (error != NS_ERROR_NONE) {
        printf("FAILED\r\n");
    } else {
        printf("PASSED\r\n");
    }

    return error;
}

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
        TEST_VERIFY_OR_EXIT(message == NULL,
                            "message queue is not empty when expected length is zero.\r\n");
    } else {
        for (message = message_queue_get_head(queue); message != NULL; message = message_get_next(message)) {
            TEST_VERIFY_OR_EXIT(expected_length != 0,
                                "message queue contains more entries than expected.\r\n");
            msg_arg = va_arg(args, message_t);
            TEST_VERIFY_OR_EXIT(msg_arg == message,
                                "message queue content does not match what is expected.\r\n");
            expected_length--;
        }
        TEST_VERIFY_OR_EXIT(expected_length == 0,
                            "message queue contains less entries than expected.\r\n");
    }

exit:
    va_end(args);
    return error;
}
