#include "ns/include/instance.h"
#include "ns/sys/core/common/instance.h"

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

}

ns_error_t
ns_message_set_offset(ns_message_t message, uint16_t offset)
{

}

bool
ns_message_is_link_security_enabled(ns_message_t message)
{

}

void
ns_message_set_direct_transmission(ns_message_t message)
{

}

int8_t
ns_message_get_rss(ns_message_t message)
{

}

ns_error_t
ns_message_append(ns_message_t message, const void *buf, uint16_t length)
{

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

}

ns_message_t
ns_message_queue_get_next(ns_message_queue_t *queue, const ns_message_t message)
{

}

void
ns_message_get_buffer_info(ns_instance_t, ns_buffer_info_t *buffer_info)
{

}
