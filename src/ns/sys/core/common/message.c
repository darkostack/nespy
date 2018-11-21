#include "ns/include/error.h"
#include "ns/include/nstd.h"
#include "ns/sys/core/common/instance.h"
#include <string.h>
#include <stdio.h>

// --- message pool functions
static message_t msg_pool_new(uint8_t type, uint16_t reserved, uint8_t priority);
static ns_error_t msg_set_length(message_t *message, uint16_t length);
static uint16_t msg_get_length(message_t *message);
static int msg_write(message_t *message, uint16_t offset, uint16_t length, void *buf);
static int msg_read(message_t *message, uint16_t offset, uint16_t length, void *buf);
static void msg_free(message_t *message);

// --- private functions
static uint16_t msg_get_free_buffer_count(instance_t *instance);
static ns_error_t msg_reclaim_buffers(instance_t *instance, int num_buffers, uint8_t priority);
static message_t *msg_new_buffer(instance_t *instance, uint8_t priority);
static uint16_t msg_get_reserved(message_t *message);
static uint8_t msg_get_priority(message_t *message);
static ns_error_t msg_resize(instance_t *instance, message_t *message, uint16_t length);

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

    // --- message pool functions
    inst->message_pool.new = msg_pool_new;
    inst->message_pool.set_length = msg_set_length;
    inst->message_pool.get_length = msg_get_length;
    inst->message_pool.write = msg_write;
    inst->message_pool.read = msg_read;
    inst->message_pool.free = msg_free;
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
    // TODO: message set priority!
    VERIFY_OR_EXIT((error = msg_set_length((message_t *)msgbuf, 0)) == NS_ERROR_NONE);

exit:
    if (error != NS_ERROR_NONE) {
        msg_free((message_t *)msgbuf);
        msgbuf = NULL;
    }
    return (message_t *)msgbuf;
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

// -------------------------------------------------------------- TEST FUNCTIONS
void message_write_read_test(void)
{
    instance_t *inst = instance_get();

    uint8_t write_buffer[1024];
    uint8_t read_buffer[1024];

    for (unsigned i = 0; i < sizeof(write_buffer); i++) {
        write_buffer[i] = i;
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
