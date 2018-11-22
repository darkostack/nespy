#include "ns/include/instance.h"
#include "ns/include/message.h"
#include "ns/sys/core/common/instance.h"

ns_message_t
ns_message_new(uint16_t type, uint16_t reserved, uint8_t priority)
{
    return message_new(type, reserved, priority);
}

void
ns_message_free(ns_message_t message)
{
    message_free((message_t)message);
}

uint16_t
ns_message_get_length(ns_message_t message)
{
    return message_get_length((message_t)message);
}

ns_error_t
ns_message_set_length(ns_message_t message, uint16_t length)
{
    return message_set_length((message_t)message, length);
}

uint16_t
ns_message_get_offset(ns_message_t message)
{
    return message_get_offset((message_t)message);
}

ns_error_t
ns_message_set_offset(ns_message_t message, uint16_t offset)
{
    return message_set_offset((message_t)message, offset);
}

bool
ns_message_is_link_security_enabled(ns_message_t message)
{
    return message_is_link_security_enabled((message_t)message);
}

void
ns_message_set_direct_transmission(ns_message_t message)
{
    message_set_direct_transmission((message_t)message);
}

int8_t
ns_message_get_rss(ns_message_t message)
{
    // TODO: add rss implementation
    return 0;
}

ns_error_t
ns_message_append(ns_message_t message, const void *buf, uint16_t length)
{
    return message_append((message_t)message, buf, length);
}

int
ns_message_read(ns_message_t message, uint16_t offset, void *buf, uint16_t length)
{
    return message_read((message_t)message, offset, buf, length);
}

int
ns_message_write(ns_message_t message, uint16_t offset, const void *buf, uint16_t length)
{
    return message_write((message_t)message, offset, buf, length);
}

void
ns_message_queue_init(ns_message_queue_t *queue)
{
    message_queue_make_new((message_queue_t *)queue);
}

ns_error_t
ns_message_queue_enqueue(ns_message_queue_t *queue, ns_message_t message)
{
    return message_queue_enqueue((message_queue_t *)queue, (message_t)message, MSG_QUEUE_POS_TAIL);
}

ns_error_t
ns_message_queue_enqueue_at_head(ns_message_t *queue, ns_message_t message)
{
    return message_queue_enqueue((message_queue_t *)queue, (message_t)message, MSG_QUEUE_POS_HEAD);
}

ns_error_t
ns_message_queue_dequeue(ns_message_t *queue, ns_message_t message)
{
    return message_queue_dequeue((message_queue_t *)queue, (message_t)message);
}

ns_message_t
ns_message_queue_get_head(ns_message_queue_t *queue)
{
    return (ns_message_t)message_queue_get_head((message_queue_t *)queue);
}

ns_message_t
ns_message_queue_get_next(ns_message_queue_t *queue, const ns_message_t message)
{
    return (ns_message_t)message_queue_get_next((message_queue_t *)queue, (const message_t)message);
}

void
ns_message_get_buffer_info(void *instance, ns_buffer_info_t *buffer_info)
{
    // TODO: get buffer information
}
