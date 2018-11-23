#include "ns/include/error.h"
#include "ns/include/nstd.h"
#include "ns/sys/core/common/instance.h"
#include "ns/test/unit/test_util.h"
#include <string.h>
#include <stdio.h>

#define NUM_NEW_PRIORITY_TEST_MESSAGES 2
#define NUM_SET_PRIORITY_TEST_MESSAGES 2
#define NUM_TEST_MESSAGES (NUM_NEW_PRIORITY_TEST_MESSAGES + NUM_SET_PRIORITY_TEST_MESSAGES)

static ns_error_t
verify_priority_queue_content(priority_queue_t *queue, int expexted_length, ...)
{
    ns_error_t error = NS_ERROR_NONE;
    va_list args;
    message_t message;
    message_t msg_arg;
    int8_t cur_priority = MSG_NUM_PRIORITIES;
    uint16_t msg_count, buf_count;

    // check priority queue get info
    message_priority_queue_get_info(queue, &msg_count, &buf_count);
    TEST_VERIFY_OR_EXIT(msg_count == expexted_length, "get info result does not match expected len.\r\n");

    va_start(args, expexted_length);

    if (expexted_length == 0) {
        message = message_priority_queue_get_head(queue);
        TEST_VERIFY_OR_EXIT(message == NULL, "priority queue is not empty when expected len is zero.\r\n");

        TEST_VERIFY_OR_EXIT(message_priority_queue_get_head_for_priority(queue, MSG_PRIO_LOW) == NULL,
                            "get head for priority non-NULL when empty\r\n");
        TEST_VERIFY_OR_EXIT(message_priority_queue_get_head_for_priority(queue, MSG_PRIO_NORMAL) == NULL,
                            "get head for priority non-NULL when empty\r\n");
        TEST_VERIFY_OR_EXIT(message_priority_queue_get_head_for_priority(queue, MSG_PRIO_HIGH) == NULL,
                            "get head for priority non-NULL when empty\r\n");
        TEST_VERIFY_OR_EXIT(message_priority_queue_get_head_for_priority(queue, MSG_PRIO_NET) == NULL,
                            "get head for priority non-NULL when empty\r\n");
    } else {
        // go through all messages in the queue and verify they match the passed-in messages
        for (message = message_priority_queue_get_head(queue);
             message != NULL;
             message = message_get_next(message)) {
            TEST_VERIFY_OR_EXIT(expexted_length != 0, "priority queue contains more entries than expected.\r\n");
            msg_arg = va_arg(args, message_t);

            if (message_get_priority(msg_arg) != cur_priority) {
                for (cur_priority--; cur_priority != message_get_priority(msg_arg); cur_priority--) {
                    // check the get head for priority is NULL if there are no
                    // expected message for this priority
                    TEST_VERIFY_OR_EXIT(
                            message_priority_queue_get_head_for_priority(queue, (uint8_t)cur_priority) == NULL,
                            "priority queue get head for priority is non-NULL when no expected msg for this priority.\r\n");
                }

                // check the get head for priority
                TEST_VERIFY_OR_EXIT(
                        message_priority_queue_get_head_for_priority(queue, (uint8_t)cur_priority) == msg_arg,
                        "priority queue get head for priority failed.\r\n");
            }

            // check the queued message to match the one from argument list
            TEST_VERIFY_OR_EXIT(msg_arg == message, "priority queue content does not match what is expected.\r\n");
            expexted_length--;
        }

        TEST_VERIFY_OR_EXIT(expexted_length == 0, "priority queue contains less entries than expected.\r\n");

        // check the get head for priority is NULL if there are no expected
        // message for any remaining priority level.
        for (cur_priority--; cur_priority >= 0; cur_priority--) {
            TEST_VERIFY_OR_EXIT(
                    message_priority_queue_get_head_for_priority(queue, (uint8_t)cur_priority) == NULL,
                    "priority queue get head for priority is non-NULL when no expected msg for this priority.\r\n");
        }
    }

    va_end(args);

exit:
    return error;
}

ns_error_t
test_message_priority_queue(void)
{
    ns_error_t error = NS_ERROR_NONE;
    //instance_t *inst = instance_get();
    priority_queue_t queue;

    message_t msg_net[NUM_TEST_MESSAGES];
    message_t msg_high[NUM_TEST_MESSAGES];
    message_t msg_nor[NUM_TEST_MESSAGES];
    message_t msg_low[NUM_TEST_MESSAGES];

    printf("---------------- TEST MESSAGE PRIORITY QUEUE\r\n");

    // initialize priority queue object
    message_priority_queue_make_new(&queue);

    // use "message_new()" function to allocate messages with different priorities
    for (int i = 0; i < NUM_NEW_PRIORITY_TEST_MESSAGES; i++) {
        msg_net[i] = message_new(0, 0, MSG_PRIO_NET);
        TEST_VERIFY_OR_EXIT(msg_net[i] != NULL, "message new failed.\r\n");
        msg_high[i] = message_new(0, 0, MSG_PRIO_HIGH);
        TEST_VERIFY_OR_EXIT(msg_high[i] != NULL, "message new failed.\r\n");
        msg_nor[i] = message_new(0, 0, MSG_PRIO_NORMAL);
        TEST_VERIFY_OR_EXIT(msg_nor[i] != NULL, "message new failed.\r\n");
        msg_low[i] = message_new(0, 0, MSG_PRIO_LOW);
        TEST_VERIFY_OR_EXIT(msg_low[i] != NULL, "message new failed.\r\n");
    }

    // check the failure case for "message_new()" for invalid argument
    TEST_VERIFY_OR_EXIT(message_new(0, 0, MSG_NUM_PRIORITIES) == NULL,
            "message new with out of range value did not fail as expected.\r\n");

    // use the "message_set_priority()" to allocate messages with different priorities
    for (int i = NUM_NEW_PRIORITY_TEST_MESSAGES; i < NUM_TEST_MESSAGES; i++) {
        msg_net[i] = message_new(0, 0, 0);
        TEST_VERIFY_OR_EXIT(msg_net[i] != NULL, "message new failed.\r\n");
        TEST_VERIFY_OR_EXIT(message_set_priority(msg_net[i], MSG_PRIO_NET) == NS_ERROR_NONE,
                            "message set priority failed.\r\n");
        msg_high[i] = message_new(0, 0, 0);
        TEST_VERIFY_OR_EXIT(msg_high[i] != NULL, "message new failed.\r\n");
        TEST_VERIFY_OR_EXIT(message_set_priority(msg_high[i], MSG_PRIO_HIGH) == NS_ERROR_NONE,
                            "message set priority failed.\r\n");
        msg_nor[i] = message_new(0, 0, 0);
        TEST_VERIFY_OR_EXIT(msg_nor[i] != NULL, "message new failed.\r\n");
        TEST_VERIFY_OR_EXIT(message_set_priority(msg_nor[i], MSG_PRIO_NORMAL) == NS_ERROR_NONE,
                            "message set priority failed.\r\n");
        msg_low[i] = message_new(0, 0, 0);
        TEST_VERIFY_OR_EXIT(msg_low[i] != NULL, "message new failed.\r\n");
        TEST_VERIFY_OR_EXIT(message_set_priority(msg_low[i], MSG_PRIO_LOW) == NS_ERROR_NONE,
                            "message set priority failed.\r\n");
    }

    // check the failure case for "message_set_priority()" for invalid argument
    TEST_VERIFY_OR_EXIT(message_set_priority(msg_net[2], MSG_NUM_PRIORITIES) == NS_ERROR_INVALID_ARGS,
                        "message set priority with out of range value did not fail as expected.\r\n");

    // check get priority
    for (int i = 0; i < NUM_TEST_MESSAGES; i++) {
        TEST_VERIFY_OR_EXIT(message_get_priority(msg_low[i]) == MSG_PRIO_LOW,
                            "message get priority failed.\r\n");
        TEST_VERIFY_OR_EXIT(message_get_priority(msg_nor[i]) == MSG_PRIO_NORMAL,
                            "message get priority failed.\r\n");
        TEST_VERIFY_OR_EXIT(message_get_priority(msg_high[i]) == MSG_PRIO_HIGH,
                            "message get priority failed.\r\n");
        TEST_VERIFY_OR_EXIT(message_get_priority(msg_net[i]) == MSG_PRIO_NET,
                            "message get priority failed.\r\n");
    }

    error = verify_priority_queue_content(&queue, 0);

    // add messages in different orders and check the content of queue
    TEST_VERIFY_OR_EXIT(message_priority_queue_enqueue(&queue, msg_high[0]) == NS_ERROR_NONE,
                        "priority queue enqueue failed.\r\n");
    error = verify_priority_queue_content(&queue, 1,
            msg_high[0]);

    TEST_VERIFY_OR_EXIT(message_priority_queue_enqueue(&queue, msg_high[1]) == NS_ERROR_NONE,
                        "priority queue enqueue failed.\r\n");
    error = verify_priority_queue_content(&queue, 2,
            msg_high[0], msg_high[1]);

    TEST_VERIFY_OR_EXIT(message_priority_queue_enqueue(&queue, msg_net[0]) == NS_ERROR_NONE,
                        "priority queue enqueue failed.\r\n");
    error = verify_priority_queue_content(&queue, 3,
            msg_net[0], msg_high[0], msg_high[1]);

    TEST_VERIFY_OR_EXIT(message_priority_queue_enqueue(&queue, msg_net[1]) == NS_ERROR_NONE,
                        "priority queue enqueue failed.\r\n");
    error = verify_priority_queue_content(&queue, 4,
            msg_net[0], msg_net[1], msg_high[0], msg_high[1]);

    TEST_VERIFY_OR_EXIT(message_priority_queue_enqueue(&queue, msg_high[2]) == NS_ERROR_NONE,
                        "priority queue enqueue failed.\r\n");
    error = verify_priority_queue_content(&queue, 5,
            msg_net[0], msg_net[1], msg_high[0], msg_high[1], msg_high[2]);

    TEST_VERIFY_OR_EXIT(message_priority_queue_enqueue(&queue, msg_low[0]) == NS_ERROR_NONE,
                        "priority queue enqueue failed.\r\n");
    error = verify_priority_queue_content(&queue, 6,
            msg_net[0], msg_net[1], msg_high[0], msg_high[1], msg_high[2], msg_low[0]);

    TEST_VERIFY_OR_EXIT(message_priority_queue_enqueue(&queue, msg_nor[0]) == NS_ERROR_NONE,
                        "priority queue enqueue failed.\r\n");
    error = verify_priority_queue_content(&queue, 7,
            msg_net[0], msg_net[1], msg_high[0], msg_high[1], msg_high[2], msg_nor[0], msg_low[0]);

    TEST_VERIFY_OR_EXIT(message_priority_queue_enqueue(&queue, msg_high[3]) == NS_ERROR_NONE,
                        "priority queue enqueue failed.\r\n");
    error = verify_priority_queue_content(&queue, 8,
            msg_net[0], msg_net[1], msg_high[0], msg_high[1], msg_high[2], msg_high[3], msg_nor[0], msg_low[0]);

exit:
    if (error != NS_ERROR_NONE) {
        printf("message priority queue test FAILED\r\n");
    } else {
        printf("message priority queue test SUCCESS\r\n");
    }

    return error;
}
