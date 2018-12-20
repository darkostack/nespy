#include "ns/include/error.h"
#include "ns/sys/core/common/instance.h"
#include "ns/sys/core/common/debug.h"
#include <string.h>

// --- private functions declarations
static void
msg_free_buffers(message_pool_t *message_pool, message_t message);

static uint16_t
msg_get_free_buffer_count(message_pool_t *message_pool);

static message_t
msg_new_buffer(message_pool_t *message_pool, uint8_t priority);

static bool
msg_is_in_queue(message_t message);

static message_t
msg_find_first_non_null_tail(priority_queue_t *queue, uint8_t start_prio_level);

static uint8_t
msg_prev_priority(uint8_t priority);

static uint16_t
msg_update_checksum_value(uint16_t checksum, uint16_t value);

static uint16_t
msg_update_checksum_buffer(uint16_t checksum, const void *buf, uint16_t length);

message_t
msg_iterator_next(message_iterator_t *iterator);

message_t
msg_iterator_prev(message_iterator_t *iterator);

// --- message pool functions
void
message_pool_ctor(message_pool_t *message_pool)
{
    memset(message_pool->buffers, 0, sizeof(message_pool->buffers));

    message_pool->free_buffers = message_pool->buffers;

    for (uint16_t i = 0; i < MSG_NUM_BUFFERS - 1; i++) {
        message_pool->buffers[i].next = (void *)&message_pool->buffers[i + 1];
    }

    message_pool->buffers[MSG_NUM_BUFFERS - 1].next = NULL;
    message_pool->num_free_buffers = MSG_NUM_BUFFERS;

    for (int prio = 0; prio < MSG_NUM_PRIORITIES; prio++) {
        message_pool->all_queue.tails[prio] = NULL;
    }
}

void
message_queue_ctor(message_queue_t *queue)
{
    queue->tail = NULL;
}

void
message_priority_queue_ctor(priority_queue_t *queue)
{
    for (int prio = 0; prio < MSG_NUM_PRIORITIES; prio++) {
        queue->tails[prio] = NULL;
    }
}

void
message_iterator_ctor(message_iterator_t *iterator)
{
    iterator->message = NULL;
}

message_t
message_new(uint8_t type, uint16_t reserved, uint8_t priority)
{
    ns_error_t error = NS_ERROR_NONE;
    message_pool_t *message_pool = &((instance_t *)instance_get())->message_pool;
    buffer_t *msgbuf = NULL;

    VERIFY_OR_EXIT((msgbuf = (buffer_t *)msg_new_buffer(message_pool, priority)) != NULL);

    memset(msgbuf, 0, sizeof(*msgbuf));
    msgbuf->buffer.head.info.message_pool = message_pool;
    msgbuf->buffer.head.info.type = type;
    msgbuf->buffer.head.info.reserved = reserved;
    msgbuf->buffer.head.info.link_security = false; // TODO: use link security

    VERIFY_OR_EXIT((error = message_set_priority((message_t)msgbuf, priority)) == NS_ERROR_NONE);
    VERIFY_OR_EXIT((error = message_set_length((message_t)msgbuf, 0)) == NS_ERROR_NONE);

exit:
    if (error != NS_ERROR_NONE) {
        message_free((message_t)msgbuf);
        msgbuf = NULL;
    }
    return (message_t)msgbuf;
}

message_t
message_new_set(uint8_t type, uint16_t reserved, const message_settings_t *settings)
{
    message_t message;
    bool link_security_enabled;
    uint8_t priority;

    if (settings == NULL) {
        link_security_enabled = true;
        priority = MSG_PRIO_NORMAL;
    } else {
        link_security_enabled = settings->link_security_enabled;
        priority = settings->priority;
    }

    message = message_new(type, reserved, priority);

    if (message != NULL) {
        message_set_link_security_enabled(message, link_security_enabled);
    }

    return message;
}

ns_error_t
message_reclaim_buffers(message_pool_t *message_pool, int num_buffers, uint8_t priority)
{
    uint16_t free_buffer_count = msg_get_free_buffer_count(message_pool);
    return (num_buffers < 0 || num_buffers <= free_buffer_count) ? NS_ERROR_NONE : NS_ERROR_NO_BUFS;
}

uint16_t
message_get_reserved(message_t message)
{
    return ((buffer_t *)message)->buffer.head.info.reserved;
}

void
message_set_reserved(message_t message, uint16_t reserved)
{
    ((buffer_t *)message)->buffer.head.info.reserved = reserved;
}

uint8_t
message_get_priority(message_t message)
{
    return ((buffer_t *)message)->buffer.head.info.priority;
}

ns_error_t
message_set_priority(message_t message, uint8_t priority)
{
    ns_error_t error = NS_ERROR_NONE;
    priority_queue_t *priority_queue = NULL;

    VERIFY_OR_EXIT(priority < MSG_NUM_PRIORITIES, error = NS_ERROR_INVALID_ARGS);
    VERIFY_OR_EXIT(msg_is_in_queue(message), ((buffer_t *)message)->buffer.head.info.priority = priority);
    VERIFY_OR_EXIT(message_get_priority(message) != priority);

    if (((buffer_t *)message)->buffer.head.info.in_priority_queue) {
        priority_queue = ((buffer_t *)message)->buffer.head.info.queue.priority;
        message_priority_queue_dequeue(priority_queue, message);
    } else {
        message_remove_from_all_queue_list(message);
    }

    ((buffer_t *)message)->buffer.head.info.priority = priority;

    if (priority_queue != NULL) {
        message_priority_queue_enqueue(priority_queue, message);
    } else {
        message_add_to_all_queue_list(message);
    }

exit:
    return error;
}

ns_error_t
message_resize(message_t message, uint16_t length)
{
    ns_error_t error = NS_ERROR_NONE;

    // add buffers
    buffer_t *cur_buffer = (buffer_t *)message;
    buffer_t *last_buffer;
    uint16_t cur_length = MSG_HEAD_BUFFER_DATA_SIZE;

    while (cur_length < length) {
        if (cur_buffer->next == NULL) {
            cur_buffer->next = (void *)msg_new_buffer(message_get_pool(message),
                                                      message_get_priority(message));
            VERIFY_OR_EXIT(cur_buffer->next != NULL, error = NS_ERROR_NO_BUFS);
        }
        cur_buffer = (buffer_t *)cur_buffer->next;
        cur_length += MSG_HEAD_BUFFER_DATA_SIZE;
    }

    // remove buffers
    last_buffer = cur_buffer;
    cur_buffer = (buffer_t *)cur_buffer->next;
    last_buffer->next = NULL;

    message_free((message_t)cur_buffer);

exit:
    return error;
}

ns_error_t
message_set_length(message_t message, uint16_t length)
{
    // NOTE: don't use freed message, it will cause invalid number of free
    // buffers in message pool because their buffer pointer isn't valid anymore.

    ns_error_t error = NS_ERROR_NONE;
    message_pool_t *message_pool = &((instance_t *)instance_get())->message_pool;

    uint16_t total_len_request = message_get_reserved(message) + length;
    uint16_t total_len_current = message_get_reserved(message) + message_get_length(message);
    int bufs = 0;

    if (total_len_request > MSG_HEAD_BUFFER_DATA_SIZE) {
        bufs = (((total_len_request - MSG_HEAD_BUFFER_DATA_SIZE) - 1) / MSG_BUFFER_DATA_SIZE) + 1;
    }

    if (total_len_current > MSG_HEAD_BUFFER_DATA_SIZE) {
        bufs -= (((total_len_current - MSG_HEAD_BUFFER_DATA_SIZE) - 1) / MSG_BUFFER_DATA_SIZE) + 1;
    }

    error = message_reclaim_buffers(message_pool, bufs, message_get_priority(message));
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);

    error = message_resize(message, total_len_request);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);

    ((buffer_t *)message)->buffer.head.info.length = length;

exit:
    return error;
}

uint16_t
message_get_length(message_t message)
{
    return ((buffer_t *)message)->buffer.head.info.length;
}

ns_error_t
message_set_offset(message_t message, uint16_t offset)
{
    ns_error_t  error = NS_ERROR_NONE;

    assert(offset <= message_get_length(message));
    VERIFY_OR_EXIT(offset <= message_get_length(message),
                   error = NS_ERROR_INVALID_ARGS);

    ((buffer_t *)message)->buffer.head.info.offset = offset;

exit:
    return error;
}

uint16_t
message_get_offset(message_t message)
{
    return ((buffer_t *)message)->buffer.head.info.offset;
}

message_pool_t *
message_get_pool(message_t message)
{
    return ((buffer_t *)message)->buffer.head.info.message_pool;
}

uint8_t
message_get_type(message_t message)
{
    return ((buffer_t *)message)->buffer.head.info.type;
}

void
message_set_type(message_t message, uint8_t type)
{
    ((buffer_t *)message)->buffer.head.info.type = type;
}

uint8_t
message_get_sub_type(message_t message)
{
    return ((buffer_t *)message)->buffer.head.info.sub_type;
}

void
message_set_sub_type(message_t message, uint8_t sub_type)
{
    ((buffer_t *)message)->buffer.head.info.sub_type = sub_type;
}

bool
message_is_sub_type_mle(message_t message)
{
    // TODO: check the message is sub type MLE or not
    return false;
}

ns_error_t
message_move_offset(message_t message, int delta)
{
    ns_error_t error = NS_ERROR_NONE;

    assert(message_get_offset(message) + delta <= message_get_length(message));

    VERIFY_OR_EXIT(message_get_offset(message) + delta <= message_get_length(message),
                   error = NS_ERROR_INVALID_ARGS);

    ((buffer_t *)message)->buffer.head.info.offset += (int16_t)delta;
    assert(((buffer_t *)message)->buffer.head.info.offset <= message_get_length(message));

exit:
    return error;
}

bool
message_is_link_security_enabled(message_t message)
{
    return ((buffer_t *)message)->buffer.head.info.link_security;
}

void
message_set_link_security_enabled(message_t message, bool enabled)
{
    ((buffer_t *)message)->buffer.head.info.link_security = enabled;
}

void
message_set_direct_transmission(message_t message)
{
    ((buffer_t *)message)->buffer.head.info.direct_tx = true;
}

void 
message_clear_direct_transmission(message_t message)
{
    ((buffer_t *)message)->buffer.head.info.direct_tx = false;
}

bool
message_get_direct_transmission(message_t message)
{
    return ((buffer_t *)message)->buffer.head.info.direct_tx;
}

void
message_set_tx_success(message_t message, bool tx_success)
{
    ((buffer_t *)message)->buffer.head.info.tx_success = tx_success;
}

bool
message_get_tx_success(message_t message)
{
    return ((buffer_t *)message)->buffer.head.info.tx_success;
}

uint16_t
message_get_datagram_tag(message_t message)
{
    return ((buffer_t *)message)->buffer.head.info.datagram_tag;
}

void
message_set_datagram_tag(message_t message, uint16_t tag)
{
    ((buffer_t *)message)->buffer.head.info.datagram_tag = tag;
}

bool
message_get_child_mask(message_t message, uint8_t child_index)
{
    assert(child_index < sizeof(((buffer_t *)message)->buffer.head.info.child_mask) * 8);
    return (((buffer_t *)message)->buffer.head.info.child_mask[child_index / 8] & (0x80 >> (child_index % 8))) != 0;
}

void
message_clear_child_mask(message_t message, uint8_t child_index)
{
    assert(child_index < sizeof(((buffer_t *)message)->buffer.head.info.child_mask) * 8);
    ((buffer_t *)message)->buffer.head.info.child_mask[child_index / 8] &= ~(0x80 >> (child_index % 8));
}

void
message_set_child_mask(message_t message, uint8_t child_index)
{
    assert(child_index < sizeof(((buffer_t *)message)->buffer.head.info.child_mask) * 8);
    ((buffer_t *)message)->buffer.head.info.child_mask[child_index / 8] |= (0x80 >> (child_index % 8));
}

bool
message_is_child_pending(message_t message)
{
    bool rval = false;
    for (size_t i = 0; i < sizeof(((buffer_t *)message)->buffer.head.info.child_mask); i++) {
        if (((buffer_t *)message)->buffer.head.info.child_mask[i] != 0) {
            EXIT_NOW(rval = true);
        }
    }
exit:
    return rval;
}

uint16_t
message_get_panid(message_t message)
{
    return ((buffer_t *)message)->buffer.head.info.panid_channel.panid;
}

void
message_set_panid(message_t message, uint16_t panid)
{
    ((buffer_t *)message)->buffer.head.info.panid_channel.panid = panid;
}

uint8_t
message_get_channel(message_t message)
{
    return ((buffer_t *)message)->buffer.head.info.panid_channel.channel;
}

void
message_set_channel(message_t message, uint8_t channel)
{
    ((buffer_t *)message)->buffer.head.info.panid_channel.channel = channel;
}

uint8_t
message_get_timeout(message_t message)
{
    return ((buffer_t *)message)->buffer.head.info.timeout;
}

void
message_set_timeout(message_t message, uint8_t timeout)
{
    ((buffer_t *)message)->buffer.head.info.timeout = timeout;
}

int8_t
message_get_interface_id(message_t message)
{
    return ((buffer_t *)message)->buffer.head.info.interface_id;
}

void
message_set_interface_id(message_t message, int8_t interface_id)
{
    ((buffer_t *)message)->buffer.head.info.interface_id = interface_id;
}

void
message_add_rss(message_t message, int8_t rss)
{
    rss_averager_add(&((buffer_t *)message)->buffer.head.info.rss_averager, rss);
}

int8_t
message_get_average_rss(message_t message)
{
    return rss_averager_get_average(&((buffer_t *)message)->buffer.head.info.rss_averager);
}

rss_averager_t *
message_get_rss_averager(message_t message)
{
    return &((buffer_t *)message)->buffer.head.info.rss_averager;
}

uint16_t
message_update_checksum(message_t message, uint16_t checksum, uint16_t offset, uint16_t length)
{
    buffer_t *cur_buffer;
    uint16_t bytes_covered = 0;
    uint16_t bytes_to_cover;

    assert(offset + length <= message_get_length(message));

    offset += message_get_reserved(message);

    // special case first buffer
    if (offset < MSG_HEAD_BUFFER_DATA_SIZE) {
        bytes_to_cover = MSG_HEAD_BUFFER_DATA_SIZE - offset;
        if (bytes_to_cover > length) {
            bytes_to_cover = length;
        }
        checksum = msg_update_checksum_buffer(checksum,
                                              ((buffer_t *)message)->buffer.head.data + offset,
                                              bytes_to_cover);
        length -= bytes_to_cover;
        bytes_covered += bytes_to_cover;
        offset = 0;
    } else {
        offset -= MSG_HEAD_BUFFER_DATA_SIZE;
    }

    // advance to offset
    cur_buffer = (buffer_t *)((buffer_t *)message)->next;

    while (offset >= MSG_BUFFER_DATA_SIZE) {
        assert(cur_buffer != NULL);
        cur_buffer = (buffer_t *)cur_buffer->next;
        offset -= MSG_BUFFER_DATA_SIZE;
    }

    // begin copy
    while (length > 0) {
        assert(cur_buffer != NULL);
        bytes_to_cover = MSG_BUFFER_DATA_SIZE - offset;

        if (bytes_to_cover > length) {
            bytes_to_cover = length;
        }

        checksum = msg_update_checksum_buffer(checksum, cur_buffer->buffer.head.data + offset, bytes_to_cover);

        length -= bytes_to_cover;
        bytes_covered += bytes_to_cover;
        cur_buffer = (buffer_t *)cur_buffer->next;
        offset = 0;
    }

    return checksum;
}

ns_error_t
message_prepend(message_t message, const void *buf, uint16_t length)
{
    ns_error_t error = NS_ERROR_NONE;
    buffer_t *new_buffer = NULL;

    while (length < message_get_reserved(message)) {
        VERIFY_OR_EXIT((new_buffer = (buffer_t *)msg_new_buffer(message_get_pool(message),
                                                                message_get_priority(message))) != NULL,
                                                                error = NS_ERROR_NO_BUFS);

        new_buffer->next = ((buffer_t *)message)->next;
        ((buffer_t *)message)->next = (void *)new_buffer;

        if (message_get_reserved(message) < sizeof(((buffer_t *)message)->buffer.head.data)) {
            // copy payload from the first buffer
            memcpy(new_buffer->buffer.head.data + message_get_reserved(message),
                   ((buffer_t *)message)->buffer.head.data + message_get_reserved(message),
                   sizeof(((buffer_t *)message)->buffer.head.data) - message_get_reserved(message));
        }

        message_set_reserved(message, message_get_reserved(message) + MSG_BUFFER_DATA_SIZE);
    }

    message_set_reserved(message, message_get_reserved(message) - length);
    ((buffer_t *)message)->buffer.head.info.length += length;
    message_set_offset(message, message_get_offset(message) + length);

    if (buf != NULL) {
        message_write(message, 0, buf, length);
    }

exit:
    return error;
}

ns_error_t
message_remove_header(message_t message, uint16_t length)
{
    assert(length <= message_get_length(message));

    ((buffer_t *)message)->buffer.head.info.reserved += length;
    ((buffer_t *)message)->buffer.head.info.length -= length;

    if (((buffer_t *)message)->buffer.head.info.offset > length) {
        ((buffer_t *)message)->buffer.head.info.offset -= length;
    } else {
        ((buffer_t *)message)->buffer.head.info.offset = 0;
    }

    return NS_ERROR_NONE;
}

ns_error_t
message_append(message_t message, const void *buf, uint16_t length)
{
    ns_error_t error = NS_ERROR_NONE;
    uint16_t old_length = message_get_length(message);
    int bytes_written;

    error = message_set_length(message, message_get_length(message) + length);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);

    bytes_written = message_write(message, old_length, buf, length);

    assert(bytes_written == (int)length);
    (void)bytes_written;

exit:
    return error;
}

int
message_write(message_t message, uint16_t offset, const void *buf, uint16_t length)
{
    buffer_t *cur_buffer;
    uint16_t bytes_copied = 0;
    uint16_t bytes_to_copy;

    assert(offset + length <= message_get_length(message));

    if (offset + length >= message_get_length(message)) {
        length = message_get_length(message) - offset;
    }

    offset += message_get_reserved(message);

    // special case first buffer
    if (offset < MSG_HEAD_BUFFER_DATA_SIZE) {
        bytes_to_copy = MSG_HEAD_BUFFER_DATA_SIZE - offset;
        if (bytes_to_copy > length) {
            bytes_to_copy = length;
        }
        memcpy(((buffer_t *)message)->buffer.head.data + offset, buf, bytes_to_copy);
        length -= bytes_to_copy;
        bytes_copied += bytes_to_copy;
        buf = (uint8_t *)buf + bytes_to_copy;
        offset = 0;
    } else {
        offset -= MSG_HEAD_BUFFER_DATA_SIZE;
    }

    // advance to offset
    cur_buffer = (buffer_t *)((buffer_t *)message)->next;

    while (offset >= MSG_BUFFER_DATA_SIZE) {
        assert(cur_buffer != NULL);
        cur_buffer = (buffer_t *)cur_buffer->next;
        offset -= MSG_BUFFER_DATA_SIZE;
    }

    // begin copy
    while (length > 0) {
        assert(cur_buffer != NULL);
        bytes_to_copy = MSG_BUFFER_DATA_SIZE - offset;
        if (bytes_to_copy > length) {
            bytes_to_copy = length;
        }
        memcpy(cur_buffer->buffer.data + offset, buf, bytes_to_copy);
        length -= bytes_to_copy;
        bytes_copied += bytes_to_copy;
        buf = (uint8_t *)buf + bytes_to_copy;

        cur_buffer = (buffer_t *)cur_buffer->next;
        offset = 0;
    }

    return bytes_copied;
}

int
message_copy_to(message_t source_message, uint16_t source_offset, uint16_t destination_offset, uint16_t length, message_t destination_message)
{
    uint16_t bytes_copied = 0;
    uint16_t bytes_to_copy;
    uint8_t buf[16];

    while (length > 0) {
        bytes_to_copy = (length < sizeof(buf)) ? length : sizeof(buf);
        message_read(source_message, source_offset, buf, bytes_to_copy);
        message_write(destination_message, destination_offset, buf, bytes_to_copy);
        source_offset += bytes_to_copy;
        destination_offset += bytes_to_copy;
        length -= bytes_to_copy;
        bytes_copied += bytes_to_copy;
    }

    return bytes_copied;
}

message_t
message_clone_length(message_t message, uint16_t length)
{
    ns_error_t error = NS_ERROR_NONE;
    message_t msgcopy;

    msgcopy = message_new(message_get_type(message),
                          message_get_reserved(message),
                          message_get_priority(message));

    VERIFY_OR_EXIT(msgcopy != NULL, error = NS_ERROR_NO_BUFS);
    VERIFY_OR_EXIT((error = message_set_length(msgcopy, length)) == NS_ERROR_NONE);

    message_copy_to(message, 0, 0, length, msgcopy);

    // copy selected information
    message_set_offset(msgcopy, message_get_offset(message));
    message_set_interface_id(msgcopy, message_get_interface_id(message));
    message_set_sub_type(msgcopy, message_get_sub_type(message));
    message_set_link_security_enabled(msgcopy, message_is_link_security_enabled(message));

exit:
    if (error != NS_ERROR_NONE && msgcopy != NULL) {
        message_free(msgcopy);
        msgcopy = NULL;
    }

    return msgcopy;
}

message_t
message_clone(message_t message)
{
    return message_clone_length(message, message_get_length(message));
}

int
message_read(message_t message, uint16_t offset, void *buf, uint16_t length)
{
    buffer_t *cur_buffer;
    uint16_t bytes_copied = 0;
    uint16_t bytes_to_copy;

    if (offset >= message_get_length(message)) {
        EXIT_NOW();
    }

    if (offset + length >= message_get_length(message)) {
        length = message_get_length(message) - offset;
    }

    offset += message_get_reserved(message);

    // special case first buffer
    if (offset < MSG_HEAD_BUFFER_DATA_SIZE) {
        bytes_to_copy = MSG_HEAD_BUFFER_DATA_SIZE - offset;
        if (bytes_to_copy > length) {
            bytes_to_copy = length;
        }
        memcpy(buf, ((buffer_t *)message)->buffer.head.data + offset, bytes_to_copy);
        length -= bytes_to_copy;
        bytes_copied += bytes_to_copy;
        buf = (uint8_t *)buf + bytes_to_copy;
        offset = 0;
    } else {
        offset -= MSG_HEAD_BUFFER_DATA_SIZE;
    }

    // advance to offset
    cur_buffer = (buffer_t *)((buffer_t *)message)->next;

    while (offset >= MSG_BUFFER_DATA_SIZE) {
        assert(cur_buffer != NULL);
        cur_buffer = (buffer_t *)cur_buffer->next;
        offset -= MSG_BUFFER_DATA_SIZE;
    }

    // begin copy
    while (length > 0) {
        assert(cur_buffer != NULL);
        bytes_to_copy = MSG_BUFFER_DATA_SIZE - offset;
        if (bytes_to_copy > length) {
            bytes_to_copy = length;
        }
        memcpy(buf, cur_buffer->buffer.data + offset, bytes_to_copy);
        length -= bytes_to_copy;
        bytes_copied += bytes_to_copy;
        buf = (uint8_t *)buf + bytes_to_copy;

        cur_buffer = (buffer_t *)cur_buffer->next;
        offset = 0;
    }

exit:
    return bytes_copied;
}

void
message_free(message_t message)
{
    message_pool_t *message_pool = &((instance_t *)instance_get())->message_pool;
    msg_free_buffers(message_pool, message);
}

message_t
message_get_next(message_t message)
{
    message_t next = NULL;
    message_t tail = NULL;

    if (((buffer_t *)message)->buffer.head.info.in_priority_queue) {
        priority_queue_t *prio_queue = ((buffer_t *)message)->buffer.head.info.queue.priority;
        VERIFY_OR_EXIT(prio_queue != NULL, next = NULL);
        tail = message_priority_queue_get_tail(prio_queue);
    } else {
        message_queue_t *msg_queue = ((buffer_t *)message)->buffer.head.info.queue.message;
        VERIFY_OR_EXIT(msg_queue != NULL, next = NULL);
        tail = (message_t)msg_queue->tail;
    }

    next = (message == tail) ? NULL : ((buffer_t *)message)->buffer.head.info.next[MSG_INFO_LIST_INTERFACE];

exit:
    return next;
}

uint8_t
message_buffer_count(message_t message)
{
    uint8_t rval = 1;
    for (buffer_t *cur_buffer = (buffer_t *)((buffer_t *)message)->next;
         cur_buffer;
         cur_buffer = cur_buffer->next) {
        rval++;
    }
    return rval;
}

void
message_remove_from_message_queue_list(message_t message, message_queue_t *queue)
{
    // this list maintained by message queue interface
    uint8_t list = MSG_INFO_LIST_INTERFACE;

    assert((((buffer_t *)message)->buffer.head.info.next[list] != NULL) &&
              (((buffer_t *)message)->buffer.head.info.prev[list] != NULL));

    if (message == (message_t)queue->tail) {
        queue->tail = (void *)((buffer_t *)queue->tail)->buffer.head.info.prev[list];
        if (message == (message_t)queue->tail) {
            queue->tail = NULL;
        }
    }

    ((buffer_t *)((buffer_t *)message)->buffer.head.info.prev[list])->buffer.head.info.next[list] =
        ((buffer_t *)message)->buffer.head.info.next[list];

    ((buffer_t *)((buffer_t *)message)->buffer.head.info.next[list])->buffer.head.info.prev[list] =
        ((buffer_t *)message)->buffer.head.info.prev[list];

    ((buffer_t *)message)->buffer.head.info.prev[list] = NULL;
    ((buffer_t *)message)->buffer.head.info.next[list] = NULL;
}

void
message_remove_from_priority_queue_list(message_t message, priority_queue_t *queue)
{
    uint8_t priority;
    buffer_t *tail;

    uint8_t list = MSG_INFO_LIST_INTERFACE;

    priority = ((buffer_t *)message)->buffer.head.info.priority;

    tail = (buffer_t *)queue->tails[priority];

    if ((buffer_t *)message == tail) {
        tail = (buffer_t *)tail->buffer.head.info.prev[list];
        if (((buffer_t *)message == tail) || (tail->buffer.head.info.priority != priority)) {
            tail = NULL;
        }
        queue->tails[priority] = (message_t)tail;
    }

    ((buffer_t *)((buffer_t *)message)->buffer.head.info.next[list])->buffer.head.info.prev[list] =
        ((buffer_t *)message)->buffer.head.info.prev[list];

    ((buffer_t *)((buffer_t *)message)->buffer.head.info.prev[list])->buffer.head.info.next[list] =
        ((buffer_t *)message)->buffer.head.info.next[list];

    ((buffer_t *)message)->buffer.head.info.next[list] = NULL;
    ((buffer_t *)message)->buffer.head.info.prev[list] = NULL;
}

void
message_remove_from_all_queue_list(message_t message)
{
    uint8_t priority;
    buffer_t *tail;
    message_pool_t *message_pool = &((instance_t *)instance_get())->message_pool;

    uint8_t list = MSG_INFO_LIST_ALL;

    priority = message_get_priority(message);

    tail = (buffer_t *)message_pool->all_queue.tails[priority];

    if ((buffer_t *)message == tail) {
        tail = (buffer_t *)tail->buffer.head.info.prev[list];
        if (((buffer_t *)message == tail) || (tail->buffer.head.info.priority != priority)) {
            tail = NULL;
        }
        message_pool->all_queue.tails[priority] = (message_t)tail;
    }

    ((buffer_t *)((buffer_t *)message)->buffer.head.info.next[list])->buffer.head.info.prev[list] =
        ((buffer_t *)message)->buffer.head.info.prev[list];

    ((buffer_t *)((buffer_t *)message)->buffer.head.info.prev[list])->buffer.head.info.next[list] =
        ((buffer_t *)message)->buffer.head.info.next[list];

    ((buffer_t *)message)->buffer.head.info.next[list] = NULL;
    ((buffer_t *)message)->buffer.head.info.prev[list] = NULL;
}

void
message_add_to_message_queue_list(message_t message, message_queue_t *queue, queue_position_t pos)
{
    // this list maintained by message queue interface
    uint8_t list = MSG_INFO_LIST_INTERFACE;

    assert((((buffer_t *)message)->buffer.head.info.next[list] == NULL) &&
              (((buffer_t *)message)->buffer.head.info.prev[list] == NULL));

    if (queue->tail == NULL) {
        ((buffer_t *)message)->buffer.head.info.next[list] = message;
        ((buffer_t *)message)->buffer.head.info.prev[list] = message;
        queue->tail = (void *)message;
    } else {
        message_t head = ((buffer_t *)queue->tail)->buffer.head.info.next[list];

        ((buffer_t *)message)->buffer.head.info.next[list] = head;
        ((buffer_t *)message)->buffer.head.info.prev[list] = (message_t)queue->tail;

        ((buffer_t *)head)->buffer.head.info.prev[list] = message;
        ((buffer_t *)queue->tail)->buffer.head.info.next[list] = message;

        if (pos == MSG_QUEUE_POS_TAIL) {
            queue->tail = (void *)message;
        }
    }
}

void
message_add_to_priority_queue_list(message_t message, priority_queue_t *queue)
{
    uint8_t priority;
    buffer_t *tail;
    buffer_t *next;

    uint8_t list = MSG_INFO_LIST_INTERFACE;

    priority = message_get_priority(message);

    tail = (buffer_t *)msg_find_first_non_null_tail(queue, priority);

    if (tail != NULL) {
        next = (buffer_t *)tail->buffer.head.info.next[list];
        ((buffer_t *)message)->buffer.head.info.next[list] = (message_t)next;
        ((buffer_t *)message)->buffer.head.info.prev[list] = (message_t)tail;
        next->buffer.head.info.prev[list] = message;
        tail->buffer.head.info.next[list] = message;
    } else {
        ((buffer_t *)message)->buffer.head.info.next[list] = message;
        ((buffer_t *)message)->buffer.head.info.prev[list] = message;
    }

    queue->tails[priority] = message;
}

void
message_add_to_all_queue_list(message_t message)
{
    uint8_t priority;
    buffer_t *tail;
    buffer_t *next;
    message_pool_t *message_pool = &((instance_t *)instance_get())->message_pool;

    uint8_t list = MSG_INFO_LIST_ALL;

    priority = message_get_priority(message);

    tail = (buffer_t *)msg_find_first_non_null_tail(&message_pool->all_queue, priority);

    if (tail != NULL) {
        next = (buffer_t *)tail->buffer.head.info.next[list];
        ((buffer_t *)message)->buffer.head.info.next[list] = (message_t)next;
        ((buffer_t *)message)->buffer.head.info.prev[list] = (message_t)tail;
        next->buffer.head.info.prev[list] = message;
        tail->buffer.head.info.next[list] = message;
    } else {
        ((buffer_t *)message)->buffer.head.info.next[list] = message;
        ((buffer_t *)message)->buffer.head.info.prev[list] = message;
    }

    message_pool->all_queue.tails[priority] = message;
}

void
message_set_message_queue(message_t message, message_queue_t *queue)
{
    ((buffer_t *)message)->buffer.head.info.queue.message = queue;
    ((buffer_t *)message)->buffer.head.info.in_priority_queue = false;
}

message_queue_t *
message_get_message_queue(message_t message)
{
    return (!((buffer_t *)message)->buffer.head.info.in_priority_queue) ?
           ((buffer_t *)message)->buffer.head.info.queue.message : NULL;
}

void
message_set_priority_queue(message_t message, priority_queue_t *queue)
{
    ((buffer_t *)message)->buffer.head.info.queue.priority = queue;
    ((buffer_t *)message)->buffer.head.info.in_priority_queue = true;
}

ns_error_t
message_queue_enqueue(message_queue_t *queue, message_t message, queue_position_t pos)
{
    ns_error_t error = NS_ERROR_NONE;

    VERIFY_OR_EXIT(!msg_is_in_queue(message), error = NS_ERROR_ALREADY);

    message_set_message_queue(message, queue);

    message_add_to_message_queue_list(message, queue, pos);
    message_add_to_all_queue_list(message);

exit:
    return error;
}

ns_error_t
message_queue_dequeue(message_queue_t *queue, message_t message)
{
    ns_error_t error = NS_ERROR_NONE;

    VERIFY_OR_EXIT(((buffer_t *)message)->buffer.head.info.queue.message == queue, error = NS_ERROR_NOT_FOUND);

    message_remove_from_message_queue_list(message, queue);
    message_remove_from_all_queue_list(message);

    message_set_message_queue(message, NULL);

exit:
    return error;
}

message_t
message_queue_get_head(message_queue_t *queue)
{
    return (queue->tail == NULL) ? NULL :
           ((buffer_t *)queue->tail)->buffer.head.info.next[MSG_INFO_LIST_INTERFACE];
}

message_t
message_queue_get_next(message_queue_t *queue, const message_t message)
{
    message_t next;
    VERIFY_OR_EXIT(message != NULL, next = NULL);
    message_queue_t *msgqueue = ((buffer_t *)message)->buffer.head.info.queue.message;
    VERIFY_OR_EXIT(msgqueue == queue, next = NULL);
    next = message_get_next(message);
exit:
    return next;
}

void
message_queue_get_info(message_queue_t *queue, uint16_t *message_count, uint16_t *buffer_count)
{
    uint16_t nmsg = 0;
    uint16_t nbuf = 0;

    for (message_t message = message_queue_get_head(queue);
         message != NULL;
         message = message_get_next(message)) {
        nmsg++;
        nbuf += message_buffer_count(message);
    }

    *message_count = nmsg;
    *buffer_count = nbuf;
}

ns_error_t
message_priority_queue_enqueue(priority_queue_t *queue, message_t message)
{
    ns_error_t error = NS_ERROR_NONE;

    VERIFY_OR_EXIT(!msg_is_in_queue(message), error = NS_ERROR_ALREADY);

    message_set_priority_queue(message, queue);

    message_add_to_priority_queue_list(message, queue);
    message_add_to_all_queue_list(message);

exit:
    return error;
}

ns_error_t
message_priority_queue_dequeue(priority_queue_t *queue, message_t message)
{
    ns_error_t error = NS_ERROR_NONE;

    VERIFY_OR_EXIT(((buffer_t *)message)->buffer.head.info.queue.priority == queue, error = NS_ERROR_NOT_FOUND);

    message_remove_from_priority_queue_list(message, queue);
    message_remove_from_all_queue_list(message);

    message_set_priority_queue(message, NULL);

exit:
    return error;
}

message_t
message_priority_queue_get_head(priority_queue_t *queue)
{
    message_t tail;
    tail = msg_find_first_non_null_tail(queue, 0);
    return (tail == NULL) ? NULL : ((buffer_t *)tail)->buffer.head.info.next[MSG_INFO_LIST_INTERFACE];
}

message_t
message_priority_queue_get_head_for_priority(priority_queue_t *queue, uint8_t priority)
{
    message_t head;
    message_t previous_tail;

    if (queue->tails[priority] != NULL) {
        previous_tail = msg_find_first_non_null_tail(queue, msg_prev_priority(priority));
        assert(previous_tail != NULL);
        head = ((buffer_t *)previous_tail)->buffer.head.info.next[MSG_INFO_LIST_INTERFACE];
    } else {
        head = NULL;
    }

    return head;
}

message_t
message_priority_queue_get_tail(priority_queue_t *queue)
{
    return msg_find_first_non_null_tail(queue, 0);
}

void
message_priority_queue_get_info(priority_queue_t *queue, uint16_t *message_count, uint16_t *buffer_count)
{
    uint16_t nmsg = 0;
    uint16_t nbuf = 0;

    for (message_t message = message_priority_queue_get_head(queue);
         message != NULL;
         message = message_get_next(message)) {
        nmsg++;
        nbuf += message_buffer_count(message);
    }

    *message_count = nmsg;
    *buffer_count = nbuf;
}

message_iterator_t *
message_iterator_get_all_messages_head(message_iterator_t *iterator)
{
    message_t head;
    message_t tail;
    message_pool_t *message_pool = &((instance_t *)instance_get())->message_pool;

    tail = message_priority_queue_get_tail(&message_pool->all_queue);

    if (tail != NULL) {
        head = ((buffer_t *)tail)->buffer.head.info.next[MSG_INFO_LIST_ALL];
    } else {
        head = NULL;
    }

    iterator->message = head;

    return iterator;
}

message_iterator_t *
message_iterator_get_all_messages_tail(message_iterator_t *iterator)
{
    message_pool_t *message_pool = &((instance_t *)instance_get())->message_pool;
    message_t tail = message_priority_queue_get_tail(&message_pool->all_queue);
    iterator->message = tail;
    return iterator;
}

message_t
message_iterator_get_message(message_iterator_t *iterator)
{
    return iterator->message;
}

bool
message_iterator_is_empty(message_iterator_t *iterator)
{
    return (iterator->message == NULL);
}

bool
message_iterator_has_ended(message_iterator_t *iterator)
{
    return message_iterator_is_empty(iterator);
}

message_iterator_t *
message_iterator_get_next(message_iterator_t *iterator)
{
    iterator->message = msg_iterator_next(iterator);
    return iterator;
}

message_iterator_t *
message_iterator_get_prev(message_iterator_t *iterator)
{
    iterator->message = msg_iterator_prev(iterator);
    return iterator;
}

// --- private functions
static void
msg_free_buffers(message_pool_t *message_pool, message_t message)
{
    while (message != NULL) {
        buffer_t *tmp_buffer = (buffer_t *)((buffer_t *)message)->next;
        ((buffer_t *)message)->next = (void *)message_pool->free_buffers;
        message_pool->free_buffers = (buffer_t *)message;
        message_pool->num_free_buffers++;
        message = (message_t)tmp_buffer;
    }
}

static uint16_t
msg_get_free_buffer_count(message_pool_t *message_pool)
{
    return message_pool->num_free_buffers;
}

static message_t
msg_new_buffer(message_pool_t *message_pool, uint8_t priority)
{
    buffer_t *buffer = NULL;
    buffer_t *free_buffers = message_pool->free_buffers;

    VERIFY_OR_EXIT(message_reclaim_buffers(message_pool, 1, priority) == NS_ERROR_NONE);

    if (free_buffers != NULL) {
        buffer = free_buffers;
        message_pool->free_buffers = (buffer_t *)free_buffers->next;
        buffer->next = NULL;
        message_pool->num_free_buffers--;
    }

exit:
    return (message_t)buffer;
}

static bool
msg_is_in_queue(message_t message)
{
    return (((buffer_t *)message)->buffer.head.info.queue.message != NULL);
}

static message_t
msg_find_first_non_null_tail(priority_queue_t *queue, uint8_t start_prio_level)
{
    message_t tail = NULL;
    uint8_t priority;

    priority = start_prio_level;

    do {
        if (queue->tails[priority] != NULL) {
            tail = queue->tails[priority];
            break;
        }
        priority = msg_prev_priority(priority);
    } while (priority != start_prio_level);

    return tail;
}

static uint8_t
msg_prev_priority(uint8_t priority)
{
    return (priority == MSG_NUM_PRIORITIES - 1) ? 0 : (priority + 1);
}

static uint16_t
msg_update_checksum_value(uint16_t checksum, uint16_t value)
{
    uint16_t result = checksum + value;
    return result + (result < checksum);
}

static uint16_t
msg_update_checksum_buffer(uint16_t checksum, const void *buf, uint16_t length)
{
    const uint8_t *bytes = (const uint8_t *)buf;
    for (int i = 0; i < length; i++) {
        checksum = msg_update_checksum_value(checksum, (i & 1) ? bytes[i] : (uint16_t)(bytes[i] << 8));
    }
    return checksum;
}

message_t
msg_iterator_next(message_iterator_t *iterator)
{
    message_t next;
    message_t tail;
    message_iterator_t it;

    VERIFY_OR_EXIT(iterator->message != NULL, next = NULL);

    it = *message_iterator_get_all_messages_tail(&it);

    tail = it.message;

    if (iterator->message == tail) {
        next = NULL;
    } else {
        next = ((buffer_t *)iterator->message)->buffer.head.info.next[MSG_INFO_LIST_ALL];
    }

exit:
    return next;
}

message_t
msg_iterator_prev(message_iterator_t *iterator)
{
    message_t prev;
    message_t head;
    message_iterator_t it;

    VERIFY_OR_EXIT(iterator->message != NULL, prev = NULL);

    it = *message_iterator_get_all_messages_head(&it);

    head = it.message;

    if (iterator->message == head) {
        prev = NULL;
    } else {
        prev = ((buffer_t *)iterator->message)->buffer.head.info.prev[MSG_INFO_LIST_ALL];
    }

exit:
    return prev;
}
