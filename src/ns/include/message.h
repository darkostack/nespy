#ifndef NS_MESSAGE_H_
#define NS_MESSAGE_H_

typedef void *ns_message_t;

typedef struct _ns_buffer_info
{
    uint16_t total_buffers;
    uint16_t free_buffers;
} ns_buffer_info_t;

typedef enum _ns_message_priority
{
    NS_MESSAGE_PRIORITY_LOW = 0,
    NS_MESSAGE_PRIORITY_NORMAL = 1,
    NS_MESSAGE_PRIORITY_HIGH = 2,
} ns_message_priority;

typedef struct _ns_message_setting
{
    bool link_security_enabled;
    ns_message_priority priority;
} ns_message_setting_t;

typedef struct _ns_message_queue
{
    void *tail;
} ns_message_queue_t;

void
ns_message_free(ns_message_t message);

uint16_t
ns_message_get_length(ns_message_t message);

ns_error_t
ns_message_set_length(ns_message_t message, uint16_t length);

uint16_t
ns_message_get_offset(ns_message_t message);

ns_error_t
ns_message_set_offset(ns_message_t message, uint16_t offset);

bool
ns_message_is_link_security_enabled(ns_message_t message);

void
ns_message_set_direct_transmission(ns_message_t message);

int8_t
ns_message_get_rss(ns_message_t message);

ns_error_t
ns_message_append(ns_message_t message, const void *buf, uint16_t length);

int
ns_message_read(ns_message_t message, uint16_t offset, void *buf, uint16_t length);

int
ns_message_write(ns_message_t message, uint16_t offset, void *buf, uint16_t length);

void
ns_message_queue_init(ns_message_queue_t *queue);

ns_error_t
ns_message_queue_enqueue(ns_message_queue_t *queue, ns_message_t message);

ns_error_t
ns_message_queue_enqueue_at_head(ns_message_t *queue, ns_message_t message);

ns_error_t
ns_message_queue_dequeue(ns_message_t *queue, ns_message_t message);

ns_message_t
ns_message_queue_get_head(ns_message_queue_t *queue);

ns_message_t
ns_message_queue_get_next(ns_message_queue_t *queue, const ns_message_t message);

void
ns_message_get_buffer_info(ns_instance_t, ns_buffer_info_t *buffer_info);

#endif // NS_MESSAGE_H_
