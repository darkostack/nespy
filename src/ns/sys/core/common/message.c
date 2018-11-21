#include "ns/include/error.h"
#include "ns/include/nstd.h"
#include "ns/sys/core/common/instance.h"
#include <string.h>
#include <stdio.h>

// --- message pool functions
static message_t msg_pool_new(uint8_t type, uint16_t reserved, uint8_t priority);
static ns_error_t msg_set_priority(message_t *message, uint8_t priority);
static ns_error_t msg_set_length(message_t *message, uint16_t length);
static uint16_t msg_get_length(message_t *message);
static int msg_write(message_t *message, uint16_t offset, uint16_t length, void *buf);
static int msg_read(message_t *message, uint16_t offset, uint16_t length, void *buf);
static void msg_free(message_t *message);
static void msg_remove_from_list(message_t *message, uint8_t list, void *queue);
static void msg_add_to_list(message_t *message, uint8_t list, void *queue, queue_position_t pos);
static void msg_set_message_queue(message_t *message, message_queue_t *queue);
static void msg_set_priority_queue(message_t *message, priority_queue_t *queue);
static ns_error_t msg_queue_enqueue(message_t *message, message_queue_t *queue, queue_position_t pos);
static ns_error_t msg_queue_dequeue(message_t *message, message_queue_t *queue);

// --- private functions
static uint16_t msg_get_free_buffer_count(instance_t *instance);
static ns_error_t msg_reclaim_buffers(instance_t *instance, int num_buffers, uint8_t priority);
static message_t *msg_new_buffer(instance_t *instance, uint8_t priority);
static uint16_t msg_get_reserved(message_t *message);
static uint8_t msg_get_priority(message_t *message);
static ns_error_t msg_resize(instance_t *instance, message_t *message, uint16_t length);
static bool msg_is_in_queue(message_t *message);
static message_t *msg_find_first_non_null_tail(uint8_t start_prio_level);
static uint8_t msg_prev_priority(uint8_t priority);

void message_pool_make_new(void *instance)
{
    instance_t *inst = (instance_t *)instance;

    memset(inst->message_pool.buffers, 0, sizeof(inst->message_pool.buffers));

    inst->message_pool.free_buffers = inst->message_pool.buffers;

    for (uint16_t i = 0; i < MSG_NUM_BUFFERS - 1; i++) {
        inst->message_pool.buffers[i].next = (void *)&inst->message_pool.buffers[i + 1];
    }

    inst->message_pool.buffers[MSG_NUM_BUFFERS - 1].next = NULL;
    inst->message_pool.num_free_buffers = MSG_NUM_BUFFERS;

    for (int prio = 0; prio < MSG_NUM_PRIORITIES; prio++) {
        inst->message_pool.all_queue.tails[prio] = NULL;
    }

    // --- message pool functions
    inst->message_pool.new = msg_pool_new;
    inst->message_pool.set_priority = msg_set_priority;
    inst->message_pool.set_length = msg_set_length;
    inst->message_pool.get_length = msg_get_length;
    inst->message_pool.write = msg_write;
    inst->message_pool.read = msg_read;
    inst->message_pool.free = msg_free;
    inst->message_pool.remove_from_list = msg_remove_from_list;
    inst->message_pool.add_to_list = msg_add_to_list;
    inst->message_pool.set_message_queue = msg_set_message_queue;
    inst->message_pool.set_priority_queue = msg_set_priority_queue;
    inst->message_pool.message_enqueue = msg_queue_enqueue;
    inst->message_pool.message_dequeue = msg_queue_dequeue;
}

// --- message pool functions
static message_t msg_pool_new(uint8_t type, uint16_t reserved, uint8_t priority)
{
    ns_error_t error = NS_ERROR_NONE;
    instance_t *inst = instance_get();
    buffer_t *msgbuf = NULL;

    VERIFY_OR_EXIT((msgbuf = (buffer_t *)msg_new_buffer(inst, priority)) != NULL);

    memset(msgbuf, 0, sizeof(*msgbuf));
    msgbuf->buffer.head.info.message_pool = (message_pool_t *)&inst->message_pool;
    msgbuf->buffer.head.info.type = type;
    msgbuf->buffer.head.info.reserved = reserved;

    VERIFY_OR_EXIT((error = msg_set_priority((message_t *)msgbuf, priority)) == NS_ERROR_NONE);
    VERIFY_OR_EXIT((error = msg_set_length((message_t *)msgbuf, 0)) == NS_ERROR_NONE);

exit:
    if (error != NS_ERROR_NONE) {
        msg_free((message_t *)msgbuf);
        msgbuf = NULL;
    }
    return (message_t *)msgbuf;
}

static ns_error_t msg_set_priority(message_t *message, uint8_t priority)
{
    ns_error_t error = NS_ERROR_NONE;
    instance_t *inst = instance_get();
    buffer_t *msgbuf = (buffer_t *)message;
    priority_queue_t *prio_queue = NULL;

    VERIFY_OR_EXIT(priority < MSG_NUM_PRIORITIES, error = NS_ERROR_INVALID_ARGS);
    VERIFY_OR_EXIT(msg_is_in_queue((message_t *)msgbuf), msgbuf->buffer.head.info.priority = priority);
    VERIFY_OR_EXIT(msgbuf->buffer.head.info.priority != priority);

    if (msgbuf->buffer.head.info.in_priority_queue) {
        // TODO: priority queue dequeue
    } else {
        inst->get_message_pool().remove_from_list((message_t *)msgbuf, MSG_INFO_LIST_ALL, NULL);
    }

    msgbuf->buffer.head.info.priority = priority;

    if (prio_queue != NULL) {
        // TODO: priority queue enqueue
    } else {
        inst->get_message_pool().add_to_list((message_t *)msgbuf, MSG_INFO_LIST_ALL, NULL, MSG_QUEUE_POS_TAIL);
    }

exit:
    return error;
}

static ns_error_t msg_set_length(message_t *message, uint16_t length)
{
    ns_error_t error = NS_ERROR_NONE;
    instance_t *inst = instance_get();
    buffer_t *msgbuf = (buffer_t *)message;

    uint16_t total_len_request = msg_get_reserved((message_t *)msgbuf) + length;
    uint16_t total_len_current = msg_get_reserved((message_t *)msgbuf) + msg_get_length((message_t *)msgbuf);
    int bufs = 0;

    if (total_len_request > MSG_HEAD_BUFFER_DATA_SIZE) {
        bufs = (((total_len_request - MSG_HEAD_BUFFER_DATA_SIZE) - 1) / MSG_BUFFER_DATA_SIZE) + 1;
    }

    if (total_len_current > MSG_HEAD_BUFFER_DATA_SIZE) {
        bufs -= (((total_len_current - MSG_HEAD_BUFFER_DATA_SIZE) - 1) / MSG_BUFFER_DATA_SIZE) + 1;
    }

    error = msg_reclaim_buffers(inst, bufs, msg_get_priority((message_t *)msgbuf));
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);

    error = msg_resize(inst, (message_t *)msgbuf, total_len_request);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);

    msgbuf->buffer.head.info.length = length;

exit:
    return error;
}

static uint16_t msg_get_length(message_t *message)
{
    buffer_t *msgbuf = (buffer_t *)message;
    return msgbuf->buffer.head.info.length;
}

static int msg_write(message_t *message, uint16_t offset, uint16_t length, void *buf)
{
    buffer_t *msgbuf = (buffer_t *)message;
    buffer_t *cur_buffer;
    uint16_t bytes_copied = 0;
    uint16_t bytes_to_copy;

    ns_assert(offset + length <= msg_get_length((message_t *)msgbuf));

    if (offset + length >= msg_get_length((message_t *)msgbuf)) {
        length = msg_get_length((message_t *)msgbuf) - offset;
    }

    offset += msg_get_reserved((message_t *)msgbuf);

    // special case first buffer
    if (offset < MSG_HEAD_BUFFER_DATA_SIZE) {
        bytes_to_copy = MSG_HEAD_BUFFER_DATA_SIZE - offset;
        if (bytes_to_copy > length) {
            bytes_to_copy = length;
        }
        memcpy(msgbuf->buffer.head.data + offset, buf, bytes_to_copy);
        length -= bytes_to_copy;
        bytes_copied += bytes_to_copy;
        buf = (uint8_t *)buf + bytes_to_copy;
        offset = 0;
    } else {
        offset -= MSG_HEAD_BUFFER_DATA_SIZE;
    }

    // advance to offset
    cur_buffer = msgbuf->next;

    while (offset >= MSG_BUFFER_DATA_SIZE) {
        ns_assert(cur_buffer != NULL);
        cur_buffer = cur_buffer->next;
        offset -= MSG_BUFFER_DATA_SIZE;
    }

    // begin copy
    while (length > 0) {
        ns_assert(cur_buffer != NULL);
        bytes_to_copy = MSG_BUFFER_DATA_SIZE - offset;
        if (bytes_to_copy > length) {
            bytes_to_copy = length;
        }
        memcpy(cur_buffer->buffer.data + offset, buf, bytes_to_copy);
        length -= bytes_to_copy;
        bytes_copied += bytes_to_copy;
        buf = (uint8_t *)buf + bytes_to_copy;

        cur_buffer = cur_buffer->next;
        offset = 0;
    }

    return bytes_copied;
}

static int msg_read(message_t *message, uint16_t offset, uint16_t length, void *buf)
{
    buffer_t *msgbuf = (buffer_t *)message;
    buffer_t *cur_buffer;
    uint16_t bytes_copied = 0;
    uint16_t bytes_to_copy;

    if (offset >= msg_get_length((message_t *)msgbuf)) {
        EXIT_NOW();
    }

    if (offset + length >= msg_get_length((message_t *)msgbuf)) {
        length = msg_get_length((message_t *)msgbuf) - offset;
    }

    offset += msg_get_reserved((message_t *)msgbuf);

    // special case first buffer
    if (offset < MSG_HEAD_BUFFER_DATA_SIZE) {
        bytes_to_copy = MSG_HEAD_BUFFER_DATA_SIZE - offset;
        if (bytes_to_copy > length) {
            bytes_to_copy = length;
        }
        memcpy(buf, msgbuf->buffer.head.data + offset, bytes_to_copy);
        length -= bytes_to_copy;
        bytes_copied += bytes_to_copy;
        buf = (uint8_t *)buf + bytes_to_copy;
        offset = 0;
    } else {
        offset -= MSG_HEAD_BUFFER_DATA_SIZE;
    }

    // advance to offset
    cur_buffer = msgbuf->next;

    while (offset >= MSG_BUFFER_DATA_SIZE) {
        ns_assert(cur_buffer != NULL);
        cur_buffer = cur_buffer->next;
        offset -= MSG_BUFFER_DATA_SIZE;
    }

    // begin copy
    while (length > 0) {
        ns_assert(cur_buffer != NULL);
        bytes_to_copy = MSG_BUFFER_DATA_SIZE - offset;
        if (bytes_to_copy > length) {
            bytes_to_copy = length;
        }
        memcpy(buf, cur_buffer->buffer.data + offset, bytes_to_copy);
        length -= bytes_to_copy;
        bytes_copied += bytes_to_copy;
        buf = (uint8_t *)buf + bytes_to_copy;

        cur_buffer = cur_buffer->next;
        offset = 0;
    }

exit:
    return bytes_copied;
}

static void msg_free(message_t *message)
{
    instance_t *inst = instance_get();
    buffer_t *msgbuf = (buffer_t *)message;
    while (msgbuf != NULL) {
        buffer_t *tmp_buffer = msgbuf->next;
        msgbuf->next = inst->message_pool.free_buffers;
        inst->message_pool.free_buffers = msgbuf;
        inst->message_pool.num_free_buffers++;
        msgbuf = tmp_buffer;
    }
}

static void msg_remove_from_list(message_t *message, uint8_t list, void *queue)
{
    instance_t *inst = instance_get();
    buffer_t *msgbuf = (buffer_t *)message;
    uint8_t priority;
    buffer_t *tail;

    // this list mantained by message pool
    if (list == MSG_INFO_LIST_ALL && queue == NULL) {

        priority = msgbuf->buffer.head.info.priority;

        tail = (buffer_t *)inst->get_message_pool().all_queue.tails[priority];

        if (msgbuf == tail) {
            tail = (buffer_t *)tail->buffer.head.info.prev[list];
            if ((msgbuf == tail) || (tail->buffer.head.info.priority != priority)) {
                tail = NULL;
            }
            inst->get_message_pool().all_queue.tails[priority] = (message_t *)tail;
        }

        ((buffer_t *)msgbuf->buffer.head.info.next[list])->buffer.head.info.prev[list] = 
            msgbuf->buffer.head.info.prev[list];

        ((buffer_t *)msgbuf->buffer.head.info.prev[list])->buffer.head.info.next[list] =
            msgbuf->buffer.head.info.next[list];

        msgbuf->buffer.head.info.next[list] = NULL;
        msgbuf->buffer.head.info.prev[list] = NULL;
    }

    // this list maintained by message queue interface
    if (list == MSG_INFO_LIST_INTERFACE && queue != NULL) {

        message_queue_t *msg_queue = (message_queue_t *)queue;

        ns_assert((msgbuf->buffer.head.info.next[list] != NULL) &&
                  (msgbuf->buffer.head.info.prev[list] != NULL));

        if (msgbuf == (buffer_t *)msg_queue->tail) {
            msg_queue->tail = ((buffer_t *)msg_queue->tail)->buffer.head.info.prev[list];
            if (msgbuf == (buffer_t *)msg_queue->tail) {
                msg_queue->tail = NULL;
            }
        }

        ((buffer_t *)msgbuf->buffer.head.info.prev[list])->buffer.head.info.next[list] =
            msgbuf->buffer.head.info.next[list];

        ((buffer_t *)msgbuf->buffer.head.info.next[list])->buffer.head.info.prev[list] =
            msgbuf->buffer.head.info.prev[list];

        msgbuf->buffer.head.info.prev[list] = NULL;
        msgbuf->buffer.head.info.next[list] = NULL;
    }
}

static void msg_add_to_list(message_t *message, uint8_t list, void *queue, queue_position_t pos)
{
    instance_t *inst = instance_get();
    buffer_t *msgbuf = (buffer_t *)message;
    uint8_t priority;
    buffer_t *tail;
    buffer_t *next;

    // this list mantained by message pool
    if (list == MSG_INFO_LIST_ALL && queue == NULL) {
        priority = msgbuf->buffer.head.info.priority;
        tail = (buffer_t *)msg_find_first_non_null_tail(priority);
        if (tail != NULL) {
            next = (buffer_t *)tail->buffer.head.info.next[list];
            msgbuf->buffer.head.info.next[list] = (message_t *)next;
            msgbuf->buffer.head.info.prev[list] = (message_t *)tail;
            next->buffer.head.info.prev[list] = (message_t *)msgbuf;
            tail->buffer.head.info.next[list] = (message_t *)msgbuf;
        } else {
            msgbuf->buffer.head.info.next[list] = (message_t *)msgbuf;
            msgbuf->buffer.head.info.prev[list] = (message_t *)msgbuf;
        }
        inst->get_message_pool().all_queue.tails[priority] = (message_t *)msgbuf;
    }

    // this list maintained by message queue interface
    if (list == MSG_INFO_LIST_INTERFACE && queue != NULL) {
        message_queue_t *msg_queue = (message_queue_t *)queue;
        ns_assert((msgbuf->buffer.head.info.next[list] == NULL) &&
                  (msgbuf->buffer.head.info.prev[list] == NULL));
        if (msg_queue->tail == NULL) {
            msgbuf->buffer.head.info.next[list] = (message_t *)msgbuf;
            msgbuf->buffer.head.info.prev[list] = (message_t *)msgbuf;
            msg_queue->tail = (message_t *)msgbuf;
        } else {
            message_t *head = ((buffer_t *)msg_queue->tail)->buffer.head.info.next[list];

            msgbuf->buffer.head.info.next[list] = head;
            msgbuf->buffer.head.info.prev[list] = msg_queue->tail;

            ((buffer_t *)head)->buffer.head.info.prev[list] = (message_t *)msgbuf;
            ((buffer_t *)msg_queue->tail)->buffer.head.info.next[list] = (message_t *)msgbuf;

            if (pos == MSG_QUEUE_POS_TAIL) {
                msg_queue->tail = (message_t *)msgbuf;
            }
        }
    }
}

static void msg_set_message_queue(message_t *message, message_queue_t *queue)
{
    buffer_t *msgbuf = (buffer_t *)message;
    msgbuf->buffer.head.info.queue.message = queue;
    msgbuf->buffer.head.info.in_priority_queue = false;
}

static void msg_set_priority_queue(message_t *message, priority_queue_t *queue)
{
    buffer_t *msgbuf = (buffer_t *)message;
    msgbuf->buffer.head.info.queue.priority = queue;
    msgbuf->buffer.head.info.in_priority_queue = true;
}

static ns_error_t msg_queue_enqueue(message_t *message, message_queue_t *queue, queue_position_t pos)
{
    ns_error_t error = NS_ERROR_NONE;
    instance_t *inst = instance_get();
    buffer_t *msgbuf = (buffer_t *)message;

    VERIFY_OR_EXIT(!msg_is_in_queue((message_t *)msgbuf), error = NS_ERROR_ALREADY);

    inst->get_message_pool().set_message_queue((message_t *)msgbuf, queue);

    inst->get_message_pool().add_to_list((message_t *)msgbuf, MSG_INFO_LIST_INTERFACE, (void *)queue, pos);
    inst->get_message_pool().add_to_list((message_t *)msgbuf, MSG_INFO_LIST_ALL, NULL, MSG_QUEUE_POS_TAIL);

exit:
    return error;
}

static ns_error_t msg_queue_dequeue(message_t *message, message_queue_t *queue)
{
    ns_error_t error = NS_ERROR_NONE;
    instance_t *inst = instance_get();
    buffer_t *msgbuf = (buffer_t *)message;

    VERIFY_OR_EXIT(msgbuf->buffer.head.info.queue.message == queue, error = NS_ERROR_NOT_FOUND);

    inst->get_message_pool().remove_from_list((message_t *)msgbuf, MSG_INFO_LIST_INTERFACE, (void *)queue);
    inst->get_message_pool().remove_from_list((message_t *)msgbuf, MSG_INFO_LIST_ALL, NULL);

    inst->get_message_pool().set_message_queue((message_t *)msgbuf, NULL);

exit:
    return error;
}

// --- private functions
static uint16_t msg_get_free_buffer_count(instance_t *instance)
{
    return instance->message_pool.num_free_buffers;
}

static ns_error_t msg_reclaim_buffers(instance_t *instance, int num_buffers, uint8_t priority)
{
    uint16_t free_buffer_count = msg_get_free_buffer_count(instance);
    return (num_buffers < 0 || num_buffers <= free_buffer_count) ? NS_ERROR_NONE : NS_ERROR_NO_BUFS;
}

static message_t *msg_new_buffer(instance_t *instance, uint8_t priority)
{
    buffer_t *buffer = NULL;
    buffer_t *free_buffers = instance->message_pool.free_buffers;

    VERIFY_OR_EXIT(msg_reclaim_buffers(instance, 1, priority) == NS_ERROR_NONE);

    if (free_buffers != NULL) {
        buffer = free_buffers;
        instance->message_pool.free_buffers = (buffer_t *)free_buffers->next;
        buffer->next = NULL;
        instance->message_pool.num_free_buffers--;
    }

exit:
    return (message_t *)buffer;
}

static uint16_t msg_get_reserved(message_t *message)
{
    buffer_t *msg = (buffer_t *)message;
    return msg->buffer.head.info.reserved;
}

static uint8_t msg_get_priority(message_t *message)
{
    buffer_t *msg = (buffer_t *)message;
    return msg->buffer.head.info.priority;
}

static ns_error_t msg_resize(instance_t *instance, message_t *message, uint16_t length)
{
    ns_error_t error = NS_ERROR_NONE;
    buffer_t *msgbuf = (buffer_t *)message;

    // add buffers
    buffer_t *cur_buffer = msgbuf;
    buffer_t *last_buffer;
    uint16_t cur_length = MSG_HEAD_BUFFER_DATA_SIZE;

    while (cur_length < length) {
        if (cur_buffer->next == NULL) {
            cur_buffer->next = (void *)msg_new_buffer(instance, msg_get_priority((message_t *)msgbuf));
            VERIFY_OR_EXIT(cur_buffer->next != NULL, error = NS_ERROR_NO_BUFS);
        }
        cur_buffer = cur_buffer->next;
        cur_length += MSG_HEAD_BUFFER_DATA_SIZE;
    }

    // remove buffers
    last_buffer = cur_buffer;
    cur_buffer = cur_buffer->next;
    last_buffer->next = NULL;

    msg_free((message_t *)cur_buffer);

exit:
    return error;
}

static bool msg_is_in_queue(message_t *message)
{
    buffer_t *msgbuf = (buffer_t *)message;
    return (msgbuf->buffer.head.info.queue.message != NULL);
}

static message_t *msg_find_first_non_null_tail(uint8_t start_prio_level)
{
    instance_t *inst = instance_get();
    message_t *tail = NULL;
    uint8_t priority;
    
    priority = start_prio_level;

    do {
        if (inst->get_message_pool().all_queue.tails[priority] != NULL) {
            tail = inst->get_message_pool().all_queue.tails[priority];
            break;
        }
        priority = msg_prev_priority(priority);
    } while (priority != start_prio_level);

    return tail;
}

static uint8_t msg_prev_priority(uint8_t priority)
{
    return (priority == MSG_NUM_PRIORITIES - 1) ? 0 : (priority + 1);
}


// -------------------------------------------------------------- TEST FUNCTIONS
void message_write_read_test(void)
{
    instance_t *inst = instance_get();

    uint8_t write_buffer[1024];
    uint8_t read_buffer[1024];

    extern uint32_t ns_plat_random_get(void);

    for (unsigned i = 0; i < sizeof(write_buffer); i++) {
        write_buffer[i] = (uint8_t)ns_plat_random_get();
    }

    message_t *message = inst->get_message_pool().new(0, 0, 0);

    printf("num of free buffers: %u\r\n", inst->get_message_pool().num_free_buffers);

    inst->get_message_pool().set_length(message, sizeof(write_buffer));
    inst->get_message_pool().write(message, 0, sizeof(write_buffer), write_buffer);
    inst->get_message_pool().read(message, 0, sizeof(read_buffer), read_buffer);

    if (memcmp(write_buffer, read_buffer, sizeof(write_buffer)) == 0) {
        printf("message write and read test SUCCESS\r\n");
    } else {
        printf("message write and read test FAILED\r\n");
    }

    printf("message get length: %u\r\n", inst->get_message_pool().get_length(message));

    inst->get_message_pool().free(message);

    printf("freed message, num of free buffers now: %u\r\n", inst->get_message_pool().num_free_buffers);
}

static message_t *msg_queue_get_head(message_queue_t *queue)
{
    return (queue->tail == NULL) ? NULL :
           ((buffer_t *)queue->tail)->buffer.head.info.next[MSG_INFO_LIST_INTERFACE];
}

static message_t *msg_get_next(message_t *message)
{
    message_t *next = NULL;
    message_t *tail = NULL;

    if (((buffer_t *)message)->buffer.head.info.in_priority_queue) {
        // TODO: priority queue
    } else {
        message_queue_t *msg_queue = ((buffer_t *)message)->buffer.head.info.queue.message;
        VERIFY_OR_EXIT(msg_queue != NULL, next = NULL);
        tail = msg_queue->tail;
    }

    next = (message == tail) ? NULL : ((buffer_t *)message)->buffer.head.info.next[MSG_INFO_LIST_INTERFACE];

exit:
    return next;
}

static uint8_t msg_get_buffer_count(message_t *message)
{
    uint8_t rval = 1;
    for (buffer_t *cur_buffer = (buffer_t *)((buffer_t *)message)->next; cur_buffer; cur_buffer = cur_buffer->next) {
        rval++;
    }
    return rval;
}

static void msg_queue_get_info(message_queue_t *queue, uint16_t *msg_count, uint16_t *buffer_count)
{
    uint16_t nmsg = 0;
    uint16_t nbuf = 0;

    for (message_t *message = msg_queue_get_head(queue); message != NULL; message = msg_get_next(message)) {
        nmsg++;
        nbuf += msg_get_buffer_count(message);
    }
    *msg_count = nmsg;
    *buffer_count = nbuf;
}

static ns_error_t verify_message_queue_content(message_queue_t *queue, int expected_length, ...)
{
    ns_error_t error = NS_ERROR_NONE;
    va_list args;
    message_t *message;
    message_t *msg_arg;

    va_start(args, expected_length);

    if (expected_length == 0) {
        message = msg_queue_get_head(queue);
        if (message != NULL) {
            printf("message queue is not empty when expected length is zero.\r\n");
            EXIT_NOW(error = NS_ERROR_FAILED);
        }
    } else {
        for (message = msg_queue_get_head(queue); message != NULL; message = msg_get_next(message)) {
            if (expected_length == 0) {
                printf("message queue contains more entries than expected.\r\n");
                EXIT_NOW(error = NS_ERROR_FAILED);
            }

            msg_arg = va_arg(args, message_t *);

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

void message_queue_test(void)
{
    uint8_t num_of_test_messages = 5;
    message_queue_t message_queue;
    message_t *msg[num_of_test_messages];
    instance_t *inst = instance_get();
    ns_error_t error = NS_ERROR_NONE;
    uint16_t msg_count, buffer_count;

    message_queue.tail = NULL;

    for (int i = 0; i < num_of_test_messages; i++) {
        msg[i] = inst->get_message_pool().new(0, 0, 0);
        if (msg[i] == NULL) {
            printf("failed to create the message!\r\n");
            EXIT_NOW();
        }
    }

    error = verify_message_queue_content(&message_queue, 0);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);

    // enqueue 1 message and remove it
    error = inst->get_message_pool().message_enqueue(msg[0], &message_queue, MSG_QUEUE_POS_TAIL);
    error = verify_message_queue_content(&message_queue, 1, msg[0]);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);

    error = inst->get_message_pool().message_dequeue(msg[0], &message_queue);
    error = verify_message_queue_content(&message_queue, 0);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);

    // enqueue 1 message at head and remove it
    error = inst->get_message_pool().message_enqueue(msg[0], &message_queue, MSG_QUEUE_POS_HEAD);
    error = verify_message_queue_content(&message_queue, 1, msg[0]);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);

    error = inst->get_message_pool().message_dequeue(msg[0], &message_queue);
    error = verify_message_queue_content(&message_queue, 0);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);

    // enqueue 5 messages
    error = inst->get_message_pool().message_enqueue(msg[0], &message_queue, MSG_QUEUE_POS_TAIL);
    error = verify_message_queue_content(&message_queue, 1, msg[0]);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);

    error = inst->get_message_pool().message_enqueue(msg[1], &message_queue, MSG_QUEUE_POS_TAIL);
    error = verify_message_queue_content(&message_queue, 2, msg[0], msg[1]);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);

    error = inst->get_message_pool().message_enqueue(msg[2], &message_queue, MSG_QUEUE_POS_TAIL);
    error = verify_message_queue_content(&message_queue, 3, msg[0], msg[1], msg[2]);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);

    error = inst->get_message_pool().message_enqueue(msg[3], &message_queue, MSG_QUEUE_POS_TAIL);
    error = verify_message_queue_content(&message_queue, 4, msg[0], msg[1], msg[2], msg[3]);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);

    error = inst->get_message_pool().message_enqueue(msg[4], &message_queue, MSG_QUEUE_POS_TAIL);
    error = verify_message_queue_content(&message_queue, 5, msg[0], msg[1], msg[2], msg[3], msg[4]);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);

    // check get info
    msg_queue_get_info(&message_queue, &msg_count, &buffer_count);
    if (msg_count != 5 ) {
        printf("ERROR: message count: %u, expect 5\r\n", msg_count);
        EXIT_NOW(error = NS_ERROR_FAILED);
    }

    // remove message in head
    error = inst->get_message_pool().message_dequeue(msg[0], &message_queue);
    error = verify_message_queue_content(&message_queue, 4, msg[1], msg[2], msg[3], msg[4]);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);

    // remove message in middle
    error = inst->get_message_pool().message_dequeue(msg[3], &message_queue);
    error = verify_message_queue_content(&message_queue, 3, msg[1], msg[2], msg[4]);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);

    // remove message from tail
    error = inst->get_message_pool().message_dequeue(msg[4], &message_queue);
    error = verify_message_queue_content(&message_queue, 2, msg[1], msg[2]);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);

    // add after remove
    error = inst->get_message_pool().message_enqueue(msg[0], &message_queue, MSG_QUEUE_POS_TAIL);
    error = verify_message_queue_content(&message_queue, 3, msg[1], msg[2], msg[0]);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);

    error = inst->get_message_pool().message_enqueue(msg[3], &message_queue, MSG_QUEUE_POS_TAIL);
    error = verify_message_queue_content(&message_queue, 4, msg[1], msg[2], msg[0], msg[3]);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);

    // remove from middle
    error = inst->get_message_pool().message_dequeue(msg[2], &message_queue);
    error = verify_message_queue_content(&message_queue, 3, msg[1], msg[0], msg[3]);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);

    // add to head
    error = inst->get_message_pool().message_enqueue(msg[2], &message_queue, MSG_QUEUE_POS_HEAD);
    error = verify_message_queue_content(&message_queue, 4, msg[2], msg[1], msg[0], msg[3]);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);

    // remove from head
    error = inst->get_message_pool().message_dequeue(msg[2], &message_queue);
    error = verify_message_queue_content(&message_queue, 3, msg[1], msg[0], msg[3]);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);

    // remove from head
    error = inst->get_message_pool().message_dequeue(msg[1], &message_queue);
    error = verify_message_queue_content(&message_queue, 2, msg[0], msg[3]);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);

    // add to head
    error = inst->get_message_pool().message_enqueue(msg[1], &message_queue, MSG_QUEUE_POS_HEAD);
    error = verify_message_queue_content(&message_queue, 3, msg[1], msg[0], msg[3]);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);

    // add to tail
    error = inst->get_message_pool().message_enqueue(msg[2], &message_queue, MSG_QUEUE_POS_TAIL);
    error = verify_message_queue_content(&message_queue, 4, msg[1], msg[0], msg[3], msg[2]);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);

    // remove all messages
    error = inst->get_message_pool().message_dequeue(msg[3], &message_queue);
    error = verify_message_queue_content(&message_queue, 3, msg[1], msg[0], msg[2]);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);

    error = inst->get_message_pool().message_dequeue(msg[1], &message_queue);
    error = verify_message_queue_content(&message_queue, 2, msg[0], msg[2]);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);

    error = inst->get_message_pool().message_dequeue(msg[2], &message_queue);
    error = verify_message_queue_content(&message_queue, 1, msg[0]);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);

    error = inst->get_message_pool().message_dequeue(msg[0], &message_queue);
    error = verify_message_queue_content(&message_queue, 0);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);

    // check the failure cases
    error = inst->get_message_pool().message_enqueue(msg[0], &message_queue, MSG_QUEUE_POS_TAIL);
    error = verify_message_queue_content(&message_queue, 1, msg[0]);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);
    // enqueue already queued message
    error = inst->get_message_pool().message_enqueue(msg[0], &message_queue, MSG_QUEUE_POS_TAIL);
    VERIFY_OR_EXIT(error == NS_ERROR_ALREADY);
    // dequeue not queued message
    error = inst->get_message_pool().message_dequeue(msg[1], &message_queue);
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