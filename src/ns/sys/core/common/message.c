#include "ns/include/error.h"
#include "ns/sys/core/common/instance.h"
#include <string.h>
#include <stdio.h>

static message_t  msg_pool_create(void *instance, uint8_t type, uint16_t reserved, uint8_t priority);

static ns_error_t msg_set_length(instance_t *instance, buffer_t *message, uint16_t length);
static uint16_t   msg_get_free_buffer_count(instance_t *instance);
static ns_error_t msg_reclaim_buffers(instance_t *instance, int num_buffers, uint8_t priority);
static buffer_t * msg_new_buffer(instance_t *instance, uint8_t priority);
static uint16_t   msg_get_reserved_len(buffer_t *message);
static uint16_t   msg_get_length(buffer_t *message);
static uint8_t    msg_get_priority(buffer_t *message);
static ns_error_t msg_resize(instance_t *instance, buffer_t *message, uint16_t length);
static void       msg_free_buffers(instance_t *instance, buffer_t *buffer);

void message_pool_make_new(void *instance)
{
    instance_t *inst = (instance_t *)instance;
    memset(inst->message_pool.buffers, 0, sizeof(inst->message_pool.buffers));
    inst->message_pool.free_buffers = inst->message_pool.buffers;
    for (uint16_t i = 0; i < MESSAGE_NUM_BUFFERS - 1; i++) {
        inst->message_pool.buffers[i].next = (void *)&inst->message_pool.buffers[i + 1];
    }
    inst->message_pool.buffers[MESSAGE_NUM_BUFFERS - 1].next = NULL;
    inst->message_pool.num_free_buffers = MESSAGE_NUM_BUFFERS;
    // assign message poll driver
    inst->message_pool.create = msg_pool_create;
}

static message_t msg_pool_create(void *instance, uint8_t type, uint16_t reserved, uint8_t priority)
{
    ns_error_t error = NS_ERROR_NONE;

    instance_t *inst = (instance_t *)instance;
    buffer_t *message = NULL;

    VERIFY_OR_EXIT((message = msg_new_buffer(inst, priority)) != NULL);

    memset(message, 0, sizeof(*message));
    message->buffer.head.info.message_pool = (message_pool_t *)&inst->message_pool;
    message->buffer.head.info.type = type;
    message->buffer.head.info.reserved = reserved;
    // TODO: message set priority!
    VERIFY_OR_EXIT((error = msg_set_length(inst, message, 0)) == NS_ERROR_NONE);

    printf("free buffers: %u\r\n", inst->message_pool.num_free_buffers);

exit:
    if (error != NS_ERROR_NONE) {
        msg_free_buffers(inst, message);
        message = NULL;
    }
    return (message_t *)message;
}

static ns_error_t msg_set_length(instance_t *instance, buffer_t *message, uint16_t length)
{
    ns_error_t error = NS_ERROR_NONE;

    uint16_t total_len_request = msg_get_reserved_len(message) + length;
    uint16_t total_len_current = msg_get_reserved_len(message) + msg_get_length(message);
    int bufs = 0;

    if (total_len_request > MESSAGE_HEAD_BUFFER_DATA_SIZE) {
        bufs = (((total_len_request - MESSAGE_HEAD_BUFFER_DATA_SIZE) - 1) / MESSAGE_BUFFER_DATA_SIZE) + 1;
    }

    if (total_len_current > MESSAGE_HEAD_BUFFER_DATA_SIZE) {
        bufs -= (((total_len_current - MESSAGE_HEAD_BUFFER_DATA_SIZE) - 1) / MESSAGE_BUFFER_DATA_SIZE) + 1;
    }

    VERIFY_OR_EXIT((error = msg_reclaim_buffers(instance, bufs, msg_get_priority(message))) == NS_ERROR_NONE);
    VERIFY_OR_EXIT((error = msg_resize(instance, message, total_len_request)) == NS_ERROR_NONE);

    message->buffer.head.info.length = length;
exit:
    return error;
}

static uint16_t msg_get_free_buffer_count(instance_t *instance)
{
    return instance->message_pool.num_free_buffers;
}

static ns_error_t msg_reclaim_buffers(instance_t *instance, int num_buffers, uint8_t priority)
{
    uint16_t free_buffer_count = msg_get_free_buffer_count(instance);
    return (num_buffers < 0 || num_buffers <= free_buffer_count) ? NS_ERROR_NONE : NS_ERROR_NO_BUFS;
}

static buffer_t *msg_new_buffer(instance_t *instance, uint8_t priority)
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
    return buffer;
}

static uint16_t msg_get_reserved_len(buffer_t *message)
{
    return message->buffer.head.info.reserved;
}

static uint16_t msg_get_length(buffer_t *message)
{
    return message->buffer.head.info.length;
}

static uint8_t msg_get_priority(buffer_t *message)
{
    return message->buffer.head.info.priority;
}

static ns_error_t msg_resize(instance_t *instance, buffer_t *message, uint16_t length)
{
    ns_error_t error = NS_ERROR_NONE;

    // add buffers
    buffer_t *cur_buffer = message;
    buffer_t *last_buffer;
    uint16_t cur_length = MESSAGE_HEAD_BUFFER_DATA_SIZE;
    while (cur_length < length) {
        if (cur_buffer->next == NULL) {
            cur_buffer->next = (void *)msg_new_buffer(instance, msg_get_priority(message));
            VERIFY_OR_EXIT(cur_buffer->next != NULL, error = NS_ERROR_NO_BUFS);
        }
        cur_buffer = cur_buffer->next;
        cur_length += MESSAGE_HEAD_BUFFER_DATA_SIZE;
    }

    // remove buffers
    last_buffer = cur_buffer;
    cur_buffer = cur_buffer->next;
    last_buffer->next = NULL;

    msg_free_buffers(instance, cur_buffer);

exit:
    return error;
}

static void msg_free_buffers(instance_t *instance, buffer_t *buffer)
{
    while (buffer != NULL) {
        buffer_t *tmp_buffer = buffer->next;
        buffer->next = instance->message_pool.free_buffers;
        instance->message_pool.free_buffers = buffer;
        instance->message_pool.num_free_buffers++;
        buffer = tmp_buffer;
    }
}
