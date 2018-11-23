#include "ns/include/error.h"
#include "ns/include/nstd.h"
#include "ns/sys/core/common/instance.h"
#include <string.h>

// --- private functions declarations
static uint16_t
msg_get_free_buffer_count(void);

static message_t
msg_new_buffer(uint8_t priority);

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

// --- message pool functions
void
message_pool_make_new(void *instance)
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
}

void
message_queue_make_new(message_queue_t *queue)
{
    queue->tail = NULL;
}

void
message_priority_queue_make_new(priority_queue_t *queue)
{
    for (int prio = 0; prio < MSG_NUM_PRIORITIES; prio++) {
        queue->tails[prio] = NULL;
    }
}

message_t
message_new(uint8_t type, uint16_t reserved, uint8_t priority)
{
    ns_error_t error = NS_ERROR_NONE;
    instance_t *inst = instance_get();
    buffer_t *msgbuf = NULL;

    VERIFY_OR_EXIT((msgbuf = (buffer_t *)msg_new_buffer(priority)) != NULL);

    memset(msgbuf, 0, sizeof(*msgbuf));
    msgbuf->buffer.head.info.message_pool = (message_pool_t *)&inst->message_pool;
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
message_reclaim_buffers(int num_buffers, uint8_t priority)
{
    uint16_t free_buffer_count = msg_get_free_buffer_count();
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
    buffer_t *msgbuf = (buffer_t *)message;
    priority_queue_t *prio_queue = NULL;

    VERIFY_OR_EXIT(priority < MSG_NUM_PRIORITIES, error = NS_ERROR_INVALID_ARGS);
    VERIFY_OR_EXIT(msg_is_in_queue((message_t)msgbuf), msgbuf->buffer.head.info.priority = priority);
    VERIFY_OR_EXIT(msgbuf->buffer.head.info.priority != priority);

    if (msgbuf->buffer.head.info.in_priority_queue) {
        prio_queue = msgbuf->buffer.head.info.queue.priority;
        message_priority_queue_dequeue(prio_queue, (message_t)msgbuf);
    } else {
        message_remove_from_all_queue_list((message_t)msgbuf);
    }

    msgbuf->buffer.head.info.priority = priority;

    if (prio_queue != NULL) {
        message_priority_queue_enqueue(prio_queue, (message_t)msgbuf);
    } else {
        message_add_to_all_queue_list((message_t)msgbuf);
    }

exit:
    return error;
}

ns_error_t
message_resize(message_t message, uint16_t length)
{
    ns_error_t error = NS_ERROR_NONE;
    buffer_t *msgbuf = (buffer_t *)message;

    // add buffers
    buffer_t *cur_buffer = msgbuf;
    buffer_t *last_buffer;
    uint16_t cur_length = MSG_HEAD_BUFFER_DATA_SIZE;

    while (cur_length < length) {
        if (cur_buffer->next == NULL) {
            cur_buffer->next = (void *)msg_new_buffer(message_get_priority((message_t)msgbuf));
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
    buffer_t *msgbuf = (buffer_t *)message;

    uint16_t total_len_request = message_get_reserved((message_t)msgbuf) + length;
    uint16_t total_len_current = message_get_reserved((message_t)msgbuf) + message_get_length((message_t)msgbuf);
    int bufs = 0;

    if (total_len_request > MSG_HEAD_BUFFER_DATA_SIZE) {
        bufs = (((total_len_request - MSG_HEAD_BUFFER_DATA_SIZE) - 1) / MSG_BUFFER_DATA_SIZE) + 1;
    }

    if (total_len_current > MSG_HEAD_BUFFER_DATA_SIZE) {
        bufs -= (((total_len_current - MSG_HEAD_BUFFER_DATA_SIZE) - 1) / MSG_BUFFER_DATA_SIZE) + 1;
    }

    error = message_reclaim_buffers(bufs, message_get_priority((message_t)msgbuf));
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);

    error = message_resize((message_t)msgbuf, total_len_request);
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);

    msgbuf->buffer.head.info.length = length;

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
    buffer_t *msgbuf = (buffer_t *)message;

    ns_assert(offset <= message_get_length((message_t)msgbuf));
    VERIFY_OR_EXIT(offset <= message_get_length((message_t)msgbuf),
                   error = NS_ERROR_INVALID_ARGS);

    msgbuf->buffer.head.info.offset = offset;

exit:
    return error;
}

uint16_t
message_get_offset(message_t message)
{
    return ((buffer_t *)message)->buffer.head.info.offset;
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

    ns_assert(message_get_offset(message) + delta <= message_get_length(message));

    VERIFY_OR_EXIT(message_get_offset(message) + delta <= message_get_length(message),
                   error = NS_ERROR_INVALID_ARGS);

    ((buffer_t *)message)->buffer.head.info.offset += (int16_t)delta;
    ns_assert(((buffer_t *)message)->buffer.head.info.offset <= message_get_length(message));

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
    buffer_t *msgbuf = (buffer_t *)message;
    ns_assert(child_index < sizeof(msgbuf->buffer.head.info.child_mask) * 8);
    return (msgbuf->buffer.head.info.child_mask[child_index / 8] & (0x80 >> (child_index % 8))) != 0;
}

void
message_clear_child_mask(message_t message, uint8_t child_index)
{
    buffer_t *msgbuf = (buffer_t *)message;
    ns_assert(child_index < sizeof(msgbuf->buffer.head.info.child_mask) * 8);
    msgbuf->buffer.head.info.child_mask[child_index / 8] &= ~(0x80 >> (child_index % 8));
}

void
message_set_child_mask(message_t message, uint8_t child_index)
{
    buffer_t *msgbuf = (buffer_t *)message;
    ns_assert(child_index < sizeof(msgbuf->buffer.head.info.child_mask) * 8);
    msgbuf->buffer.head.info.child_mask[child_index / 8] |= (0x80 >> (child_index % 8));
}

bool
message_is_child_pending(message_t message)
{
    bool rval = false;
    buffer_t *msgbuf = (buffer_t *)message;
    for (size_t i = 0; i < sizeof(msgbuf->buffer.head.info.child_mask); i++) {
        if (msgbuf->buffer.head.info.child_mask[i] != 0) {
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

/* TODO:
void
message_add_rss(message_t message, int8_t rss)
{

}

int8_t
message_get_average_rss(message_t message)
{

}

rss_averager_t *
message_get_rss_averager(message_t message)
{

}
*/

uint16_t
message_update_checksum(message_t message, uint16_t checksum, uint16_t offset, uint16_t length)
{
    buffer_t *msgbuf = (buffer_t *)message;
    buffer_t *cur_buffer;
    uint16_t bytes_covered = 0;
    uint16_t bytes_to_cover;

    ns_assert(offset + length <= message_get_length((message_t)msgbuf));

    offset += message_get_reserved((message_t)msgbuf);

    // special case first buffer
    if (offset < MSG_HEAD_BUFFER_DATA_SIZE) {
        bytes_to_cover = MSG_HEAD_BUFFER_DATA_SIZE - offset;
        if (bytes_to_cover > length) {
            bytes_to_cover = length;
        }
        checksum = msg_update_checksum_buffer(checksum, msgbuf->buffer.head.data + offset, bytes_to_cover);
        length -= bytes_to_cover;
        bytes_covered += bytes_to_cover;
        offset = 0;
    } else {
        offset -= MSG_HEAD_BUFFER_DATA_SIZE;
    }

    // advance to offset
    cur_buffer = (buffer_t *)msgbuf->next;

    while (offset >= MSG_BUFFER_DATA_SIZE) {
        ns_assert(cur_buffer != NULL);
        cur_buffer = (buffer_t *)cur_buffer->next;
        offset -= MSG_BUFFER_DATA_SIZE;
    }

    // begin copy
    while (length > 0) {
        ns_assert(cur_buffer != NULL);
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
    buffer_t *msgbuf = (buffer_t *)message;
    buffer_t *new_buffer = NULL;

    while (length < message_get_reserved(message)) {
        VERIFY_OR_EXIT((new_buffer = (buffer_t *)msg_new_buffer(message_get_priority((message_t)msgbuf))) != NULL, error = NS_ERROR_NO_BUFS);

        new_buffer->next = (void *)msgbuf->next;
        msgbuf->next = (void *)new_buffer;

        if (message_get_reserved((message_t)msgbuf) < sizeof(msgbuf->buffer.head.data)) {
            // copy payload from the first buffer
            memcpy(new_buffer->buffer.head.data + message_get_reserved((message_t)msgbuf),
                   msgbuf->buffer.head.data + message_get_reserved((message_t)msgbuf),
                   sizeof(msgbuf->buffer.head.data) - message_get_reserved((message_t)msgbuf));
        }

        message_set_reserved((message_t)msgbuf, message_get_reserved((message_t)msgbuf) + MSG_BUFFER_DATA_SIZE);
    }

    message_set_reserved((message_t)msgbuf, message_get_reserved((message_t)msgbuf) - length);
    msgbuf->buffer.head.info.length += length;
    message_set_offset((message_t)msgbuf, message_get_offset((message_t)msgbuf) + length);

    if (buf != NULL) {
        message_write((message_t)msgbuf, 0, buf, length);
    }

exit:
    return error;
}

ns_error_t
message_remove_header(message_t message, uint16_t length)
{
    buffer_t *msgbuf = (buffer_t *)message;

    ns_assert(length <= msgbuf->buffer.head.info.length);

    msgbuf->buffer.head.info.reserved += length;
    msgbuf->buffer.head.info.length -= length;

    if (msgbuf->buffer.head.info.offset > length) {
        msgbuf->buffer.head.info.offset -= length;
    } else {
        msgbuf->buffer.head.info.offset = 0;
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

    ns_assert(bytes_written == (int)length);
    (void)bytes_written;

exit:
    return error;
}

int
message_write(message_t message, uint16_t offset, const void *buf, uint16_t length)
{
    buffer_t *msgbuf = (buffer_t *)message;
    buffer_t *cur_buffer;
    uint16_t bytes_copied = 0;
    uint16_t bytes_to_copy;

    ns_assert(offset + length <= message_get_length((message_t)msgbuf));

    if (offset + length >= message_get_length((message_t)msgbuf)) {
        length = message_get_length((message_t)msgbuf) - offset;
    }

    offset += message_get_reserved((message_t)msgbuf);

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
    cur_buffer = (buffer_t *)msgbuf->next;

    while (offset >= MSG_BUFFER_DATA_SIZE) {
        ns_assert(cur_buffer != NULL);
        cur_buffer = (buffer_t *)cur_buffer->next;
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
    buffer_t *msgbuf = (buffer_t *)message;
    buffer_t *cur_buffer;
    uint16_t bytes_copied = 0;
    uint16_t bytes_to_copy;

    if (offset >= message_get_length((message_t)msgbuf)) {
        EXIT_NOW();
    }

    if (offset + length >= message_get_length((message_t)msgbuf)) {
        length = message_get_length((message_t)msgbuf) - offset;
    }

    offset += message_get_reserved((message_t)msgbuf);

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
    cur_buffer = (buffer_t *)msgbuf->next;

    while (offset >= MSG_BUFFER_DATA_SIZE) {
        ns_assert(cur_buffer != NULL);
        cur_buffer = (buffer_t *)cur_buffer->next;
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

        cur_buffer = (buffer_t *)cur_buffer->next;
        offset = 0;
    }

exit:
    return bytes_copied;
}

void
message_free(message_t message)
{
    instance_t *inst = instance_get();
    buffer_t *msgbuf = (buffer_t *)message;
    while (msgbuf != NULL) {
        buffer_t *tmp_buffer = (buffer_t *)msgbuf->next;
        msgbuf->next = (void *)inst->message_pool.free_buffers;
        inst->message_pool.free_buffers = msgbuf;
        inst->message_pool.num_free_buffers++;
        msgbuf = tmp_buffer;
    }
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
    for (buffer_t *cur_buffer = (buffer_t *)((buffer_t *)message)->next; cur_buffer; cur_buffer = cur_buffer->next) {
        rval++;
    }
    return rval;
}

void
message_remove_from_message_queue_list(message_t message, message_queue_t *queue)
{
    buffer_t *msgbuf = (buffer_t *)message;

    // this list maintained by message queue interface
    uint8_t list = MSG_INFO_LIST_INTERFACE;

    ns_assert((msgbuf->buffer.head.info.next[list] != NULL) &&
              (msgbuf->buffer.head.info.prev[list] != NULL));

    if (msgbuf == (buffer_t *)queue->tail) {
        queue->tail = (void *)((buffer_t *)queue->tail)->buffer.head.info.prev[list];
        if (msgbuf == (buffer_t *)queue->tail) {
            queue->tail = NULL;
        }
    }

    ((buffer_t *)msgbuf->buffer.head.info.prev[list])->buffer.head.info.next[list] =
        msgbuf->buffer.head.info.next[list];

    ((buffer_t *)msgbuf->buffer.head.info.next[list])->buffer.head.info.prev[list] =
        msgbuf->buffer.head.info.prev[list];

    msgbuf->buffer.head.info.prev[list] = NULL;
    msgbuf->buffer.head.info.next[list] = NULL;
}

void
message_remove_from_priority_queue_list(message_t message, priority_queue_t *queue)
{
    buffer_t *msgbuf = (buffer_t *)message;
    uint8_t priority;
    buffer_t *tail;

    uint8_t list = MSG_INFO_LIST_INTERFACE;

    priority = msgbuf->buffer.head.info.priority;

    tail = queue->tails[priority];

    if (msgbuf == tail) {
        tail = (buffer_t *)tail->buffer.head.info.prev[list];
        if ((msgbuf == tail) || (tail->buffer.head.info.priority != priority)) {
            tail = NULL;
        }
        queue->tails[priority] = (message_t)tail;
    }

    ((buffer_t *)msgbuf->buffer.head.info.next[list])->buffer.head.info.prev[list] =
        msgbuf->buffer.head.info.prev[list];

    ((buffer_t *)msgbuf->buffer.head.info.prev[list])->buffer.head.info.next[list] =
        msgbuf->buffer.head.info.next[list];

    msgbuf->buffer.head.info.next[list] = NULL;
    msgbuf->buffer.head.info.prev[list] = NULL;
}

void
message_remove_from_all_queue_list(message_t message)
{
    instance_t *inst = instance_get();
    buffer_t *msgbuf = (buffer_t *)message;
    uint8_t priority;
    buffer_t *tail;

    uint8_t list = MSG_INFO_LIST_ALL;

    priority = msgbuf->buffer.head.info.priority;

    tail = (buffer_t *)inst->message_pool.all_queue.tails[priority];

    if (msgbuf == tail) {
        tail = (buffer_t *)tail->buffer.head.info.prev[list];
        if ((msgbuf == tail) || (tail->buffer.head.info.priority != priority)) {
            tail = NULL;
        }
        inst->message_pool.all_queue.tails[priority] = (message_t)tail;
    }

    ((buffer_t *)msgbuf->buffer.head.info.next[list])->buffer.head.info.prev[list] =
        msgbuf->buffer.head.info.prev[list];

    ((buffer_t *)msgbuf->buffer.head.info.prev[list])->buffer.head.info.next[list] =
        msgbuf->buffer.head.info.next[list];

    msgbuf->buffer.head.info.next[list] = NULL;
    msgbuf->buffer.head.info.prev[list] = NULL;
}

void
message_add_to_message_queue_list(message_t message, message_queue_t *queue, queue_position_t pos)
{
    buffer_t *msgbuf = (buffer_t *)message;

    // this list maintained by message queue interface
    uint8_t list = MSG_INFO_LIST_INTERFACE;

    ns_assert((msgbuf->buffer.head.info.next[list] == NULL) &&
              (msgbuf->buffer.head.info.prev[list] == NULL));

    if (queue->tail == NULL) {
        msgbuf->buffer.head.info.next[list] = (message_t)msgbuf;
        msgbuf->buffer.head.info.prev[list] = (message_t)msgbuf;
        queue->tail = (void *)msgbuf;
    } else {
        message_t head = ((buffer_t *)queue->tail)->buffer.head.info.next[list];

        msgbuf->buffer.head.info.next[list] = head;
        msgbuf->buffer.head.info.prev[list] = (message_t)queue->tail;

        ((buffer_t *)head)->buffer.head.info.prev[list] = (message_t)msgbuf;
        ((buffer_t *)queue->tail)->buffer.head.info.next[list] = (message_t)msgbuf;

        if (pos == MSG_QUEUE_POS_TAIL) {
            queue->tail = (void *)msgbuf;
        }
    }
}

void
message_add_to_priority_queue_list(message_t message, priority_queue_t *queue)
{
    buffer_t *msgbuf = (buffer_t *)message;
    uint8_t priority;
    buffer_t *tail;
    buffer_t *next;

    uint8_t list = MSG_INFO_LIST_INTERFACE;

    priority = msgbuf->buffer.head.info.priority;

    tail = (buffer_t *)msg_find_first_non_null_tail(queue, priority);

    if (tail != NULL) {
        next = (buffer_t *)tail->buffer.head.info.next[list];
        msgbuf->buffer.head.info.next[list] = (message_t)next;
        msgbuf->buffer.head.info.prev[list] = (message_t)tail;
        next->buffer.head.info.prev[list] = (message_t)msgbuf;
        tail->buffer.head.info.next[list] = (message_t)msgbuf;
    } else {
        msgbuf->buffer.head.info.next[list] = (message_t)msgbuf;
        msgbuf->buffer.head.info.prev[list] = (message_t)msgbuf;
    }

    queue->tails[priority] = (message_t)msgbuf;
}

void
message_add_to_all_queue_list(message_t message)
{
    instance_t *inst = instance_get();
    buffer_t *msgbuf = (buffer_t *)message;
    uint8_t priority;
    buffer_t *tail;
    buffer_t *next;

    uint8_t list = MSG_INFO_LIST_ALL;

    priority = msgbuf->buffer.head.info.priority;

    tail = (buffer_t *)msg_find_first_non_null_tail(&inst->message_pool.all_queue, priority);

    if (tail != NULL) {
        next = (buffer_t *)tail->buffer.head.info.next[list];
        msgbuf->buffer.head.info.next[list] = (message_t)next;
        msgbuf->buffer.head.info.prev[list] = (message_t)tail;
        next->buffer.head.info.prev[list] = (message_t)msgbuf;
        tail->buffer.head.info.next[list] = (message_t)msgbuf;
    } else {
        msgbuf->buffer.head.info.next[list] = (message_t)msgbuf;
        msgbuf->buffer.head.info.prev[list] = (message_t)msgbuf;
    }

    inst->message_pool.all_queue.tails[priority] = (message_t)msgbuf;
}

void
message_set_message_queue(message_t message, message_queue_t *queue)
{
    buffer_t *msgbuf = (buffer_t *)message;
    msgbuf->buffer.head.info.queue.message = queue;
    msgbuf->buffer.head.info.in_priority_queue = false;
}

message_queue_t *
message_get_message_queue(message_t message)
{
    buffer_t *msgbuf = (buffer_t *)message;
    return (!msgbuf->buffer.head.info.in_priority_queue) ? msgbuf->buffer.head.info.queue.message : NULL;
}

void
message_set_priority_queue(message_t message, priority_queue_t *queue)
{
    buffer_t *msgbuf = (buffer_t *)message;
    msgbuf->buffer.head.info.queue.priority = queue;
    msgbuf->buffer.head.info.in_priority_queue = true;
}

ns_error_t
message_queue_enqueue(message_queue_t *queue, message_t message, queue_position_t pos)
{
    ns_error_t error = NS_ERROR_NONE;
    buffer_t *msgbuf = (buffer_t *)message;

    VERIFY_OR_EXIT(!msg_is_in_queue((message_t)msgbuf), error = NS_ERROR_ALREADY);

    message_set_message_queue((message_t)msgbuf, queue);

    message_add_to_message_queue_list((message_t)msgbuf, queue, pos);
    message_add_to_all_queue_list((message_t)msgbuf);

exit:
    return error;
}

ns_error_t
message_queue_dequeue(message_queue_t *queue, message_t message)
{
    ns_error_t error = NS_ERROR_NONE;
    buffer_t *msgbuf = (buffer_t *)message;

    VERIFY_OR_EXIT(msgbuf->buffer.head.info.queue.message == queue, error = NS_ERROR_NOT_FOUND);

    message_remove_from_message_queue_list((message_t)msgbuf, queue);
    message_remove_from_all_queue_list((message_t)msgbuf);

    message_set_message_queue((message_t)msgbuf, NULL);

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

    for (message_t message = message_queue_get_head(queue); message != NULL; message = message_get_next(message)) {
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
    buffer_t *msgbuf = (buffer_t *)message;

    VERIFY_OR_EXIT(!msg_is_in_queue((message_t)msgbuf), error = NS_ERROR_ALREADY);

    message_set_priority_queue((message_t)msgbuf, queue);

    message_add_to_priority_queue_list((message_t)msgbuf, queue);
    message_add_to_all_queue_list((message_t)msgbuf);

exit:
    return error;
}

ns_error_t
message_priority_queue_dequeue(priority_queue_t *queue, message_t message)
{
    ns_error_t error = NS_ERROR_NONE;
    buffer_t *msgbuf = (buffer_t *)message;

    VERIFY_OR_EXIT(msgbuf->buffer.head.info.queue.priority == queue, error = NS_ERROR_NOT_FOUND);

    message_remove_from_priority_queue_list((message_t)msgbuf, queue);
    message_remove_from_all_queue_list((message_t)msgbuf);

    message_set_priority_queue((message_t)msgbuf, NULL);

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
        ns_assert(previous_tail != NULL);
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

// --- private functions
static uint16_t
msg_get_free_buffer_count(void)
{
    instance_t *inst = instance_get();
    return inst->message_pool.num_free_buffers;
}

static message_t
msg_new_buffer(uint8_t priority)
{
    instance_t *inst = instance_get();
    buffer_t *buffer = NULL;
    buffer_t *free_buffers = inst->message_pool.free_buffers;

    VERIFY_OR_EXIT(message_reclaim_buffers(1, priority) == NS_ERROR_NONE);

    if (free_buffers != NULL) {
        buffer = free_buffers;
        inst->message_pool.free_buffers = (buffer_t *)free_buffers->next;
        buffer->next = NULL;
        inst->message_pool.num_free_buffers--;
    }

exit:
    return (message_t)buffer;
}

static bool
msg_is_in_queue(message_t message)
{
    buffer_t *msgbuf = (buffer_t *)message;
    return (msgbuf->buffer.head.info.queue.message != NULL);
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
