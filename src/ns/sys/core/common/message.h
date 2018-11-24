#ifndef NS_CORE_COMMON_MESSAGE_H_
#define NS_CORE_COMMON_MESSAGE_H_

#include "ns/include/message.h"

typedef ns_message_t message_t;
typedef struct _buffer buffer_t;
typedef struct _message_info message_info_t;
typedef struct _message_pool message_pool_t;
typedef ns_message_queue_t message_queue_t;
typedef struct _priority_queue priority_queue_t;
typedef ns_message_settings_t message_settings_t;

enum {
    MSG_INFO_LIST_ALL       = 0, // identifies the all messages list (maintained by the message_pool)
    MSG_INFO_LIST_INTERFACE = 1, // identifies the list for per-interface message queue
    MSG_INFO_NUM_LISTS      = 2, // number of list
};

struct _message_info {
    message_t next[MSG_INFO_NUM_LISTS]; // a pointer to the next message in doubly linked list
    message_t prev[MSG_INFO_NUM_LISTS]; // a pointer to the previous message ind doubly linked list
    message_pool_t *message_pool;       // identifies the message pool for this message

    union {
        message_queue_t *message;       // identifies the message queue (if any) where this message queued
        priority_queue_t *priority;     // identifies the priority queue (if any) where this message queued
    } queue;                            // identifies the queue (if any) where this message queued

    uint16_t reserved;                  // number of header byte reserved for the message
    uint16_t length;                    // number of bytes within the message
    uint16_t offset;                    // a byte offset within the message
    uint16_t datagram_tag;              // the datagram tag used for 6LoWPAN fragmentation
    // TODO: rss_averager_t rss_averager;        // the averager maintaining the received signal strength (RSS) average

    uint8_t child_mask[8];              // a bit-vector to indicate which sleepy children need to receive this
    uint8_t timeout;                    // seconds remaining before dropping the message
    int8_t interface_id;                // the interface id
    union {
        uint16_t panid;                 // used for MLE discover request and response messages
        uint8_t channel;                // used for MLE announce
    } panid_channel;                    // used for MLE discover request, response, and announce messages

    uint8_t type : 2;                   // identifies the type of message
    uint8_t sub_type : 4;               // identifies the message sub type
    bool direct_tx : 1;                 // used to indicate whether a direct transmission is required
    bool link_security : 1;             // indicates whether or not link security is enabled
    uint8_t priority : 2;               // indicates the message priority level
    bool in_priority_queue : 1;         // indicates whether the message is queued in normal or priority queue
    bool tx_success : 1;                // indicates whether the direct tx of the message was successful
};

enum {
    MSG_NUM_BUFFERS           = NS_CONFIG_MESSAGE_NUM_BUFFERS,
    MSG_BUFFER_SIZE           = NS_CONFIG_MESSAGE_BUFFER_SIZE,
    MSG_BUFFER_DATA_SIZE      = MSG_BUFFER_SIZE - sizeof(void *), // (void *) is reserved for next pointer
    MSG_HEAD_BUFFER_DATA_SIZE = MSG_BUFFER_DATA_SIZE - sizeof(struct _message_info),
};

// Note: the sizeof _buffer structure must equal to MESSAGE_BUFFER_SIZE
struct _buffer {
    union {
        struct {
            message_info_t info;
            uint8_t data[MSG_HEAD_BUFFER_DATA_SIZE];
        } head;
        uint8_t data[MSG_BUFFER_DATA_SIZE];
    } buffer;
    void *next;
};

enum {
    MSG_PRIO_LOW    = NS_MESSAGE_PRIORITY_LOW,
    MSG_PRIO_NORMAL = NS_MESSAGE_PRIORITY_NORMAL,
    MSG_PRIO_HIGH   = NS_MESSAGE_PRIORITY_HIGH,
    MSG_PRIO_NET    = NS_MESSAGE_PRIORITY_HIGH + 1,
    MSG_NUM_PRIORITIES = 4,
};

typedef enum _queue_position {
    MSG_QUEUE_POS_HEAD,
    MSG_QUEUE_POS_TAIL,
} queue_position_t;

struct _priority_queue {
    message_t tails[MSG_NUM_PRIORITIES];
};

struct _message_pool {
    uint16_t num_free_buffers;
    buffer_t buffers[MSG_NUM_BUFFERS];
    buffer_t *free_buffers;
    priority_queue_t all_queue;
};

void
message_pool_make_new(void *instance);

void
message_queue_make_new(message_queue_t *queue);

void
message_priority_queue_make_new(priority_queue_t *queue);

message_t
message_new(uint8_t type, uint16_t reserved, uint8_t priority);

message_t
message_new_set(uint8_t type, uint16_t reserved, const message_settings_t *settings);

ns_error_t
message_reclaim_buffers(int num_buffers, uint8_t priority);

uint16_t
message_get_reserved(message_t message);

void
message_set_reserved(message_t message, uint16_t reserved);

uint8_t
message_get_priority(message_t message);

ns_error_t
message_set_priority(message_t message, uint8_t priority);

ns_error_t
message_resize(message_t message, uint16_t length);

ns_error_t
message_set_length(message_t message, uint16_t length);

uint16_t
message_get_length(message_t message);

ns_error_t
message_set_offset(message_t message, uint16_t offset);

uint16_t
message_get_offset(message_t message);

uint8_t
message_get_type(message_t message);

void
message_set_type(message_t message, uint8_t type);

uint8_t
message_get_sub_type(message_t message);

void
message_set_sub_type(message_t message, uint8_t sub_type);

bool
message_is_sub_type_mle(message_t message);

ns_error_t
message_move_offset(message_t message, int delta);

bool
message_is_link_security_enabled(message_t message);

void
message_set_link_security_enabled(message_t message, bool enabled);

void
message_set_direct_transmission(message_t message);

void 
message_clear_direct_transmission(message_t message);

bool
message_get_direct_transmission(message_t message);

void
message_set_tx_success(message_t message, bool tx_success);

bool
message_get_tx_success(message_t message);

uint16_t
message_get_datagram_tag(message_t message);

void
message_set_datagram_tag(message_t message, uint16_t tag);

bool
message_get_child_mask(message_t message, uint8_t child_index);

void
message_clear_child_mask(message_t message, uint8_t child_index);

void
message_set_child_mask(message_t message, uint8_t child_index);

bool
message_is_child_pending(message_t message);

uint16_t
message_get_panid(message_t message);

void
message_set_panid(message_t message, uint16_t panid);

uint8_t
message_get_channel(message_t message);

void
message_set_channel(message_t message, uint8_t channel);

uint8_t
message_get_timeout(message_t message);

void
message_set_timeout(message_t message, uint8_t timeout);

int8_t
message_get_interface_id(message_t message);

void
message_set_interface_id(message_t message, int8_t interface_id);

/* TODO:
void
message_add_rss(message_t message, int8_t rss);

int8_t
message_get_average_rss(message_t message);

rss_averager_t *
message_get_rss_averager(message_t message);
*/

uint16_t
message_update_checksum(message_t message, uint16_t checksum, uint16_t offset, uint16_t length);

ns_error_t
message_prepend(message_t message, const void *buf, uint16_t length);

ns_error_t
message_remove_header(message_t message, uint16_t length);

ns_error_t
message_append(message_t message, const void *buf, uint16_t length);

int
message_write(message_t message, uint16_t offset, const void *buf, uint16_t length);

int
message_copy_to(message_t source_message, uint16_t source_offset, uint16_t destination_offset, uint16_t length, message_t destination_message);

message_t
message_clone_length(message_t message, uint16_t length);

message_t
message_clone(message_t message);

int
message_read(message_t message, uint16_t offset, void *buf, uint16_t length);

void
message_free(message_t message);

message_t
message_get_next(message_t message);

uint8_t
message_buffer_count(message_t message);

void
message_remove_from_message_queue_list(message_t message, message_queue_t *queue);

void
message_remove_from_priority_queue_list(message_t message, priority_queue_t *queue);

void
message_remove_from_all_queue_list(message_t message);

void
message_add_to_message_queue_list(message_t message, message_queue_t *queue, queue_position_t pos);

void
message_add_to_priority_queue_list(message_t message, priority_queue_t *queue);

void
message_add_to_all_queue_list(message_t message);

void
message_set_message_queue(message_t message, message_queue_t *queue);

message_queue_t *
message_get_message_queue(message_t message);

void
message_set_priority_queue(message_t message, priority_queue_t *queue);

ns_error_t
message_queue_enqueue(message_queue_t *queue, message_t message, queue_position_t pos);

ns_error_t
message_queue_dequeue(message_queue_t *queue, message_t message);

message_t
message_queue_get_head(message_queue_t *queue);

message_t
message_queue_get_next(message_queue_t *queue, const message_t message);

void
message_queue_get_info(message_queue_t *queue, uint16_t *message_count, uint16_t *buffer_count);

ns_error_t
message_priority_queue_enqueue(priority_queue_t *queue, message_t message);

ns_error_t
message_priority_queue_dequeue(priority_queue_t *queue, message_t message);

message_t
message_priority_queue_get_head(priority_queue_t *queue);

message_t
message_priority_queue_get_head_for_priority(priority_queue_t *queue, uint8_t priority);

message_t
message_priority_queue_get_tail(priority_queue_t *queue);

void
message_priority_queue_get_info(priority_queue_t *queue, uint16_t *message_count, uint16_t *buffer_count);

#endif // NS_CORE_COMMON_MESSAGE_H_
