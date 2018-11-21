#ifndef NS_CORE_COMMON_MESSAGE_H_
#define NS_CORE_COMMON_MESSAGE_H_

typedef void *message_t;
typedef struct _buffer buffer_t;
typedef struct _message_info message_info_t;
typedef struct _message_pool message_pool_t;
typedef struct _message_queue message_queue_t;
typedef struct _priority_queue priority_queue_t;

enum {
    MSG_INFO_LIST_ALL       = 0, // identifies the all messages list (maintained by the message_pool)
    MSG_INFO_LIST_INTERFACE = 1, // identifies the list for per-interface message queue
    MSG_INFO_NUM_LISTS      = 2, // number of list
};

struct _message_info {
    message_t next[MSG_INFO_NUM_LISTS]; // a pointer to the next message in doubly linked list
    message_t prev[MSG_INFO_NUM_LISTS]; // a pointer to the previous message ind doubly linked list
    message_pool_t *message_pool;

    union {
        message_queue_t *message;
        priority_queue_t *priority;
    } queue;

    uint16_t reserved;
    uint16_t length;
    uint16_t offset;

    uint8_t type : 2;
    uint8_t sub_type : 4;
    uint8_t priority : 2;
    bool in_priority_queue : 1;
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
    MSG_PRIO_LOW,
    MSG_PRIO_NORMAL,
    MSG_PRIO_HIGH,
    MSG_PRIO_NET,
    MSG_NUM_PRIORITIES = 4,
};

typedef enum _queue_position {
    MSG_QUEUE_POS_HEAD,
    MSG_QUEUE_POS_TAIL,
} queue_position_t;

struct _message_queue {
    void *tail;
};

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

message_t
message_new(uint8_t type, uint16_t reserved, uint8_t priority);

ns_error_t
message_reclaim_buffers(int num_buffers, uint8_t priority);

uint16_t
message_get_reserved(message_t message);

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

int
message_write(message_t message, uint16_t offset, const void *buf, uint16_t length);

int
message_read(message_t message, uint16_t offset, void *buf, uint16_t length);

void
message_free(message_t message);

void
message_remove_from_list(message_t message, uint8_t list, void *queue);

void
message_add_to_list(message_t message, uint8_t list, void *queue, queue_position_t pos);

void
message_set_message_queue(message_t message, message_queue_t *queue);

void
message_set_priority_queue(message_t message, priority_queue_t *queue);

ns_error_t
message_queue_enqueue(message_queue_t *queue, message_t message, queue_position_t pos);

ns_error_t
message_queue_dequeue(message_queue_t *queue, message_t message);

#endif // NS_CORE_COMMON_MESSAGE_H_
