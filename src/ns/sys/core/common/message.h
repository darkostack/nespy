#ifndef NS_CORE_COMMON_MESSAGE_H_
#define NS_CORE_COMMON_MESSAGE_H_

typedef void *message_t;
typedef struct _buffer buffer_t;
typedef struct _message_info message_info_t;
typedef struct _message_pool message_pool_t;

enum {
    MESSAGE_INFO_LIST_ALL = 0, // identifies the all messages list (maintained by the message_pool)
    MESSAGE_INFO_LIST_INTERFACE = 1, // identifies the list for per-interface message queue
    MESSAGE_INFO_NUM_LISTS = 2, // number of list
};


struct _message_info {
    message_t *next[MESSAGE_INFO_NUM_LISTS]; // a pointer to the next message in doubly linked list
    message_t *prev[MESSAGE_INFO_NUM_LISTS]; // a pointer to the previous message ind doubly linked list
    message_pool_t *message_pool;
    uint16_t reserved;
    uint16_t length;
    uint16_t offset;
    uint8_t type;
    uint8_t sub_type;
    uint8_t priority;
};

enum {
    MESSAGE_NUM_BUFFERS = NS_CONFIG_MESSAGE_NUM_BUFFERS,
    MESSAGE_BUFFER_SIZE = NS_CONFIG_MESSAGE_BUFFER_SIZE,
    MESSAGE_BUFFER_DATA_SIZE = MESSAGE_BUFFER_SIZE - sizeof(void *), // (void *) is reserved for next pointer
    MESSAGE_HEAD_BUFFER_DATA_SIZE = MESSAGE_BUFFER_DATA_SIZE - sizeof(struct _message_info),
};

// Note: the sizeof _buffer structure must equal to MESSAGE_BUFFER_SIZE
struct _buffer {
    union {
        struct {
            message_info_t info;
            uint8_t data[MESSAGE_HEAD_BUFFER_DATA_SIZE];
        } head;
        uint8_t data[MESSAGE_BUFFER_DATA_SIZE];
    } buffer;
    void *next;
};

struct _message_pool {
    uint16_t num_free_buffers;
    buffer_t buffers[MESSAGE_NUM_BUFFERS];
    buffer_t *free_buffers;
    // message pool driver
    message_t (* create)(void *instance, uint8_t type, uint16_t reserved, uint8_t priority);
};

void message_pool_make_new(void *instance);

#endif // NS_CORE_COMMON_MESSAGE_H_
