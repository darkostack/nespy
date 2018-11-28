#include "ns/include/error.h"
#include "ns/include/nstd.h"
#include "ns/sys/core/common/instance.h"
#include "ns/test/unit/test_util.h"
#include <string.h>
#include <stdio.h>

#define NUM_NEW_PRIORITY_TEST_MESSAGES 2
#define NUM_SET_PRIORITY_TEST_MESSAGES 2
#define NUM_TEST_MESSAGES (NUM_NEW_PRIORITY_TEST_MESSAGES + NUM_SET_PRIORITY_TEST_MESSAGES)

// this function verifies the content of the priority queue to match the passed in messages
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

exit:
    va_end(args);
    return error;
}

// this function verifies the content of the all message in queue to match the passed in messages
static ns_error_t
verify_all_messages_content(int expected_length, ...)
{
    ns_error_t error = NS_ERROR_NONE;
    va_list args;
    message_iterator_t it;
    message_t msg_arg;

    va_start(args, expected_length);

    if (expected_length == 0) {
        it = *message_iterator_get_all_messages_head(&it);
        TEST_VERIFY_OR_EXIT(message_iterator_is_empty(&it),
                            "head is not empty when expected len is zero.\r\n");
        it = *message_iterator_get_all_messages_tail(&it);
        TEST_VERIFY_OR_EXIT(message_iterator_is_empty(&it),
                            "tail is not empty when expected len is zero.\r\n");
    } else {
        for (it = *message_iterator_get_all_messages_head(&it);
             !message_iterator_has_ended(&it);
             it = *message_iterator_get_next(&it)) {
            TEST_VERIFY_OR_EXIT(expected_length != 0,
                                "all messages queue contains more entries than expected.\r\n");
            msg_arg = va_arg(args, message_t);
            TEST_VERIFY_OR_EXIT(msg_arg == message_iterator_get_message(&it),
                                "all messages queue content does not match what is expected.\r\n");
            expected_length--;
        }

        TEST_VERIFY_OR_EXIT(expected_length == 0,
                            "all messages queue contains less entries than expeced.\r\n");
    }

exit:
    va_end(args);
    return error;
}

// this function verifies the content of the all message in queue to match the
// passed int messages. it goes through the all messages list in reverse
static ns_error_t
verify_all_messages_content_in_reverse(int expected_length, ...)
{
    ns_error_t error = NS_ERROR_NONE;
    va_list args;
    message_iterator_t it;
    message_t msg_arg;

    va_start(args, expected_length);

    if (expected_length == 0) {
        it = *message_iterator_get_all_messages_head(&it);
        TEST_VERIFY_OR_EXIT(message_iterator_is_empty(&it),
                            "head is not empty when expected len is zero.\r\n");
        it = *message_iterator_get_all_messages_tail(&it);
        TEST_VERIFY_OR_EXIT(message_iterator_is_empty(&it),
                            "tail is not empty when expected len is zero.\r\n");
    } else {
        for (it = *message_iterator_get_all_messages_tail(&it);
             !message_iterator_has_ended(&it);
             it = *message_iterator_get_prev(&it)) {
            TEST_VERIFY_OR_EXIT(expected_length != 0,
                                "all messages queue contains more entries than expected.\r\n");
            msg_arg = va_arg(args, message_t);
            TEST_VERIFY_OR_EXIT(msg_arg == message_iterator_get_message(&it),
                                "all messages queue content does not match what is expected.\r\n");
            expected_length--;
        }
    }

exit:
    va_end(args);
    return error;
}

// this function verifies the content of the message queue to match the passed in messages
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

ns_error_t
test_message_priority_queue(void)
{
    ns_error_t error = NS_ERROR_NONE;
    instance_t *inst = instance_get();
    priority_queue_t queue;
    message_queue_t message_queue;
    message_iterator_t it;

    message_t msg_net[NUM_TEST_MESSAGES];
    message_t msg_high[NUM_TEST_MESSAGES];
    message_t msg_nor[NUM_TEST_MESSAGES];
    message_t msg_low[NUM_TEST_MESSAGES];

    printf("---------------- TEST MESSAGE PRIORITY QUEUE\r\n");

    // call objects contructor
    message_priority_queue_ctor(&queue);
    message_iterator_ctor(&it);
    message_queue_ctor(&message_queue);

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
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);

    // add messages in different orders and check the content of queue
    TEST_VERIFY_OR_EXIT(message_priority_queue_enqueue(&queue, msg_high[0]) == NS_ERROR_NONE,
                        "priority queue enqueue failed.\r\n");
    error = verify_priority_queue_content(&queue, 1,
            msg_high[0]);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);

    TEST_VERIFY_OR_EXIT(message_priority_queue_enqueue(&queue, msg_high[1]) == NS_ERROR_NONE,
                        "priority queue enqueue failed.\r\n");
    error = verify_priority_queue_content(&queue, 2,
            msg_high[0], msg_high[1]);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);

    TEST_VERIFY_OR_EXIT(message_priority_queue_enqueue(&queue, msg_net[0]) == NS_ERROR_NONE,
                        "priority queue enqueue failed.\r\n");
    error = verify_priority_queue_content(&queue, 3,
            msg_net[0], msg_high[0], msg_high[1]);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);

    TEST_VERIFY_OR_EXIT(message_priority_queue_enqueue(&queue, msg_net[1]) == NS_ERROR_NONE,
                        "priority queue enqueue failed.\r\n");
    error = verify_priority_queue_content(&queue, 4,
            msg_net[0], msg_net[1], msg_high[0], msg_high[1]);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);

    TEST_VERIFY_OR_EXIT(message_priority_queue_enqueue(&queue, msg_high[2]) == NS_ERROR_NONE,
                        "priority queue enqueue failed.\r\n");
    error = verify_priority_queue_content(&queue, 5,
            msg_net[0], msg_net[1], msg_high[0], msg_high[1], msg_high[2]);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);

    TEST_VERIFY_OR_EXIT(message_priority_queue_enqueue(&queue, msg_low[0]) == NS_ERROR_NONE,
                        "priority queue enqueue failed.\r\n");
    error = verify_priority_queue_content(&queue, 6,
            msg_net[0], msg_net[1], msg_high[0], msg_high[1], msg_high[2], msg_low[0]);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);

    TEST_VERIFY_OR_EXIT(message_priority_queue_enqueue(&queue, msg_nor[0]) == NS_ERROR_NONE,
                        "priority queue enqueue failed.\r\n");
    error = verify_priority_queue_content(&queue, 7,
            msg_net[0], msg_net[1], msg_high[0], msg_high[1], msg_high[2], msg_nor[0], msg_low[0]);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);

    TEST_VERIFY_OR_EXIT(message_priority_queue_enqueue(&queue, msg_high[3]) == NS_ERROR_NONE,
                        "priority queue enqueue failed.\r\n");
    error = verify_priority_queue_content(&queue, 8,
            msg_net[0], msg_net[1], msg_high[0], msg_high[1], msg_high[2], msg_high[3], msg_nor[0], msg_low[0]);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);

    // check message iterator methods
    TEST_VERIFY_OR_EXIT(message_iterator_is_empty(&it),
                        "iterator is empty failed to return `true` for an empty iterator\r\n");

    it = *message_iterator_get_next(&it);
    TEST_VERIFY_OR_EXIT(message_iterator_is_empty(&it),
                        "iterator is empty failed to return `true` for an empty iterator\r\n");

    it = *message_iterator_get_next(&it);
    TEST_VERIFY_OR_EXIT(message_iterator_is_empty(&it),
                        "iterator is empty failed to return `true` for an empty iterator\r\n");

    it = *message_iterator_get_prev(&it);
    TEST_VERIFY_OR_EXIT(message_iterator_is_empty(&it),
                        "iterator is empty failed to return `true` for an empty iterator\r\n");

    it = *message_iterator_get_all_messages_head(&it);
    TEST_VERIFY_OR_EXIT(!message_iterator_is_empty(&it),
                        "iterator is empty failed to return `false` when it's not empty\r\n");

    TEST_VERIFY_OR_EXIT(message_iterator_get_message(&it) == msg_net[0],
                        "iterator get message failed.\r\n");

    it = *message_iterator_get_next(&it);
    TEST_VERIFY_OR_EXIT(!message_iterator_is_empty(&it),
                        "iterator is empty failed to return `false` when it's not empty\r\n");
    TEST_VERIFY_OR_EXIT(message_iterator_get_message(&it) == msg_net[1],
                        "iterator get message failed.\r\n");

    it = *message_iterator_get_prev(&it);
    TEST_VERIFY_OR_EXIT(!message_iterator_is_empty(&it),
                        "iterator is empty failed to return `false` when it's not empty\r\n");
    TEST_VERIFY_OR_EXIT(message_iterator_get_message(&it) == msg_net[0],
                        "iterator get message failed.\r\n");

    it = *message_iterator_get_prev(&it);
    TEST_VERIFY_OR_EXIT(message_iterator_has_ended(&it),
                        "iterator goto prev failed to return empty at head.\r\n");

    it = *message_iterator_get_all_messages_tail(&it);
    it = *message_iterator_get_next(&it);
    TEST_VERIFY_OR_EXIT(message_iterator_has_ended(&it),
                        "iterator goto next failed to return empty at tail.\r\n");

    // check all messages queue content (should match the content of priority queue)
    error = verify_all_messages_content(8,
            msg_net[0], msg_net[1], msg_high[0], msg_high[1], msg_high[2], msg_high[3], msg_nor[0], msg_low[0]);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);

    error = verify_all_messages_content_in_reverse(8,
            msg_low[0], msg_nor[0], msg_high[3], msg_high[2], msg_high[1], msg_high[0], msg_net[1], msg_net[0]);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);

    // remove messages in different order and check the content of queue in each step
    TEST_VERIFY_OR_EXIT(message_priority_queue_dequeue(&queue, msg_net[0]) == NS_ERROR_NONE,
                        "priority queue dequeue failed.\r\n");
    error = verify_priority_queue_content(&queue, 7,
            msg_net[1], msg_high[0], msg_high[1], msg_high[2], msg_high[3], msg_nor[0], msg_low[0]);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);

    TEST_VERIFY_OR_EXIT(message_priority_queue_dequeue(&queue, msg_high[2]) == NS_ERROR_NONE,
                        "priority queue dequeue failed.\r\n");
    error = verify_priority_queue_content(&queue, 6,
            msg_net[1], msg_high[0], msg_high[1], msg_high[3], msg_nor[0], msg_low[0]);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);

    TEST_VERIFY_OR_EXIT(message_priority_queue_dequeue(&queue, msg_nor[0]) == NS_ERROR_NONE,
                        "priority queue dequeue failed.\r\n");
    error = verify_priority_queue_content(&queue, 5,
            msg_net[1], msg_high[0], msg_high[1], msg_high[3], msg_low[0]);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);

    TEST_VERIFY_OR_EXIT(message_priority_queue_dequeue(&queue, msg_low[0]) == NS_ERROR_NONE,
                        "priority queue dequeue failed.\r\n");
    error = verify_priority_queue_content(&queue, 4,
            msg_net[1], msg_high[0], msg_high[1], msg_high[3]);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);

    TEST_VERIFY_OR_EXIT(message_priority_queue_dequeue(&queue, msg_net[1]) == NS_ERROR_NONE,
                        "priority queue dequeue failed.\r\n");
    error = verify_priority_queue_content(&queue, 3,
            msg_high[0], msg_high[1], msg_high[3]);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);

    TEST_VERIFY_OR_EXIT(message_priority_queue_dequeue(&queue, msg_high[0]) == NS_ERROR_NONE,
                        "priority queue dequeue failed.\r\n");
    error = verify_priority_queue_content(&queue, 2,
            msg_high[1], msg_high[3]);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);

    TEST_VERIFY_OR_EXIT(message_priority_queue_dequeue(&queue, msg_high[3]) == NS_ERROR_NONE,
                        "priority queue dequeue failed.\r\n");
    error = verify_priority_queue_content(&queue, 1,
            msg_high[1]);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);

    TEST_VERIFY_OR_EXIT(message_priority_queue_dequeue(&queue, msg_high[1]) == NS_ERROR_NONE,
                        "priority queue dequeue failed.\r\n");
    error = verify_priority_queue_content(&queue, 0);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);

    // check the failure cases: enqueuing an already queued message, or
    // dequeueing a message not queued
    TEST_VERIFY_OR_EXIT(message_priority_queue_enqueue(&queue, msg_net[0]) == NS_ERROR_NONE,
                        "priority queue enqueue failed.\r\n");
    error = verify_priority_queue_content(&queue, 1, msg_net[0]);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);

    TEST_VERIFY_OR_EXIT(message_priority_queue_enqueue(&queue, msg_net[0]) == NS_ERROR_ALREADY,
                        "enqueuing an alread queued message did not fail as expected.\r\n");
    TEST_VERIFY_OR_EXIT(message_priority_queue_dequeue(&queue, msg_high[0]) == NS_ERROR_NOT_FOUND,
                        "dequeueing a message not queued, did not fail as expected.\r\n");

    TEST_VERIFY_OR_EXIT(message_priority_queue_dequeue(&queue, msg_net[0]) == NS_ERROR_NONE,
                        "priority queue dequeue failed.\r\n");
    error = verify_priority_queue_content(&queue, 0);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);

    // change the priority of an already queued message and check the order change in the queue
    TEST_VERIFY_OR_EXIT(message_priority_queue_enqueue(&queue, msg_nor[0]) == NS_ERROR_NONE,
                        "priority queue enqueue failed.\r\n");
    error = verify_priority_queue_content(&queue, 1, msg_nor[0]);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);

    TEST_VERIFY_OR_EXIT(message_priority_queue_enqueue(&queue, msg_high[0]) == NS_ERROR_NONE,
                        "priority queue enqueue failed.\r\n");
    error = verify_priority_queue_content(&queue, 2, msg_high[0], msg_nor[0]);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);

    TEST_VERIFY_OR_EXIT(message_priority_queue_enqueue(&queue, msg_low[0]) == NS_ERROR_NONE,
                        "priority queue enqueue failed.\r\n");
    error = verify_priority_queue_content(&queue, 3, msg_high[0], msg_nor[0], msg_low[0]);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);

    TEST_VERIFY_OR_EXIT(message_set_priority(msg_nor[0], MSG_PRIO_NET) == NS_ERROR_NONE,
                        "set priority failed for an already queued message.\r\n");
    error = verify_priority_queue_content(&queue, 3, msg_nor[0], msg_high[0], msg_low[0]);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);

    TEST_VERIFY_OR_EXIT(message_set_priority(msg_low[0], MSG_PRIO_LOW) == NS_ERROR_NONE,
                        "set priority failed for an already queued message.\r\n");
    error = verify_priority_queue_content(&queue, 3, msg_nor[0], msg_high[0], msg_low[0]);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);

    TEST_VERIFY_OR_EXIT(message_set_priority(msg_low[0], MSG_PRIO_NORMAL) == NS_ERROR_NONE,
                        "set priority failed for an already queued message.\r\n");
    error = verify_priority_queue_content(&queue, 3, msg_nor[0], msg_high[0], msg_low[0]);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);

    TEST_VERIFY_OR_EXIT(message_set_priority(msg_low[0], MSG_PRIO_HIGH) == NS_ERROR_NONE,
                        "set priority failed for an already queued message.\r\n");
    error = verify_priority_queue_content(&queue, 3, msg_nor[0], msg_high[0], msg_low[0]);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);

    TEST_VERIFY_OR_EXIT(message_set_priority(msg_low[0], MSG_PRIO_NET) == NS_ERROR_NONE,
                        "set priority failed for an already queued message.\r\n");
    error = verify_priority_queue_content(&queue, 3, msg_nor[0], msg_low[0], msg_high[0]);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);

    TEST_VERIFY_OR_EXIT(message_set_priority(msg_nor[0], MSG_PRIO_NORMAL) == NS_ERROR_NONE,
                        "set priority failed for an already queued message.\r\n");
    TEST_VERIFY_OR_EXIT(message_set_priority(msg_low[0], MSG_PRIO_LOW) == NS_ERROR_NONE,
                        "set priority failed for an already queued message.\r\n");

    error = verify_priority_queue_content(&queue, 3, msg_high[0], msg_nor[0], msg_low[0]);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);

    error = verify_all_messages_content(3, msg_high[0], msg_nor[0], msg_low[0]);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);

    error = verify_all_messages_content_in_reverse(3, msg_low[0], msg_nor[0], msg_high[0]);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);

    // checking the all messages queue when adding messages frome same pool to another queue
    TEST_VERIFY_OR_EXIT(message_queue_enqueue(&message_queue, msg_nor[1], MSG_QUEUE_POS_TAIL) == NS_ERROR_NONE,
                        "message queue enqueue failed.\r\n");
    error = verify_all_messages_content(4,
            msg_high[0], msg_nor[0], msg_nor[1], msg_low[0]);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);

    TEST_VERIFY_OR_EXIT(message_queue_enqueue(&message_queue, msg_high[1], MSG_QUEUE_POS_TAIL) == NS_ERROR_NONE,
                        "message queue enqueue failed.\r\n");
    error = verify_all_messages_content(5,
            msg_high[0], msg_high[1], msg_nor[0], msg_nor[1], msg_low[0]);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);

    TEST_VERIFY_OR_EXIT(message_queue_enqueue(&message_queue, msg_net[1], MSG_QUEUE_POS_TAIL) == NS_ERROR_NONE,
                        "message queue enqueue failed.\r\n");
    error = verify_all_messages_content(6,
            msg_net[1], msg_high[0], msg_high[1], msg_nor[0], msg_nor[1], msg_low[0]);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);

    error = verify_message_queue_content(&message_queue, 3,
            msg_nor[1], msg_high[1], msg_net[1]);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);

    // change priority of message and check that order changes in the all
    // messages queue and not in message queue
    TEST_VERIFY_OR_EXIT(message_set_priority(msg_nor[1], MSG_PRIO_NET) == NS_ERROR_NONE,
                        "message set priority failed for an already queued message.\r\n");
    error = verify_all_messages_content(6,
            msg_net[1], msg_nor[1], msg_high[0], msg_high[1], msg_nor[0], msg_low[0]);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);
    error = verify_all_messages_content_in_reverse(6,
            msg_low[0], msg_nor[0], msg_high[1], msg_high[0], msg_nor[1], msg_net[1]);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);
    error = verify_message_queue_content(&message_queue, 3,
            msg_nor[1], msg_high[1], msg_net[1]);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);

    TEST_VERIFY_OR_EXIT(message_set_priority(msg_low[0], MSG_PRIO_HIGH) == NS_ERROR_NONE,
                        "message set priority failed for an already queued message.\r\n");
    error = verify_all_messages_content(6,
            msg_net[1], msg_nor[1], msg_high[0], msg_high[1], msg_low[0], msg_nor[0]);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);
    error = verify_all_messages_content_in_reverse(6,
            msg_nor[0], msg_low[0], msg_high[1], msg_high[0], msg_nor[1], msg_net[1]);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);
    error = verify_priority_queue_content(&queue, 3,
            msg_high[0], msg_low[0], msg_nor[0]);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);
    error = verify_message_queue_content(&message_queue, 3,
            msg_nor[1], msg_high[1], msg_net[1]);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);

    // remove messages from the two queues and verify that all message queue is updated correctly
    TEST_VERIFY_OR_EXIT(message_priority_queue_dequeue(&queue, msg_high[0]) == NS_ERROR_NONE,
                        "priority queue dequeue failed.\r\n");
    error = verify_all_messages_content(5,
            msg_net[1], msg_nor[1], msg_high[1], msg_low[0], msg_nor[0]);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);
    error = verify_priority_queue_content(&queue, 2,
            msg_low[0], msg_nor[0]);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);
    error = verify_message_queue_content(&message_queue, 3,
            msg_nor[1], msg_high[1], msg_net[1]);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);

    TEST_VERIFY_OR_EXIT(message_queue_dequeue(&message_queue, msg_net[1]) == NS_ERROR_NONE,
                        "message queue dequeue failed.\r\n");
    error = verify_all_messages_content(4,
            msg_nor[1], msg_high[1], msg_low[0], msg_nor[0]);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);
    error = verify_priority_queue_content(&queue, 2,
            msg_low[0], msg_nor[0]);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);
    error = verify_message_queue_content(&message_queue, 2,
            msg_nor[1], msg_high[1]);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);

    TEST_VERIFY_OR_EXIT(message_queue_dequeue(&message_queue, msg_high[1]) == NS_ERROR_NONE,
                        "message queue dequeue failed.\r\n");
    error = verify_all_messages_content(3,
            msg_nor[1], msg_low[0], msg_nor[0]);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);
    error = verify_priority_queue_content(&queue, 2,
            msg_low[0], msg_nor[0]);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);
    error = verify_message_queue_content(&message_queue, 1,
            msg_nor[1]);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);

    TEST_VERIFY_OR_EXIT(message_priority_queue_dequeue(&queue, msg_low[0]) == NS_ERROR_NONE,
                        "priority queue dequeue failed.\r\n");
    error = verify_all_messages_content(2,
            msg_nor[1], msg_nor[0]);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);
    error = verify_priority_queue_content(&queue, 1,
            msg_nor[0]);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);
    error = verify_message_queue_content(&message_queue, 1,
            msg_nor[1]);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);

    TEST_VERIFY_OR_EXIT(message_priority_queue_dequeue(&queue, msg_nor[0]) == NS_ERROR_NONE,
                        "priority queue dequeue failed.\r\n");
    error = verify_all_messages_content(1,
            msg_nor[1]);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);
    error = verify_priority_queue_content(&queue, 0);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);
    error = verify_message_queue_content(&message_queue, 1,
            msg_nor[1]);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);

    TEST_VERIFY_OR_EXIT(message_queue_dequeue(&message_queue, msg_nor[1]) == NS_ERROR_NONE,
                        "message queue dequeue failed.\r\n");
    error = verify_all_messages_content(0);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);
    error = verify_priority_queue_content(&queue, 0);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);
    error = verify_message_queue_content(&message_queue, 0);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);

    // free all the messages
    for (int i = 0; i < NUM_TEST_MESSAGES; i++) {
        message_free(msg_net[i]);
        message_free(msg_high[i]);
        message_free(msg_nor[i]);
        message_free(msg_low[i]);
    }

    TEST_VERIFY_OR_EXIT(inst->message_pool.num_free_buffers == MSG_NUM_BUFFERS,
                        "message free buffers failed, expected 44 num of free buffer.\r\n");

exit:
    if (error != NS_ERROR_NONE) {
        printf("FAILED\r\n");
    } else {
        printf("PASSED\r\n");
    }

    return error;
}
