#ifndef NS_CORE_COMMON_MESSAGE_H_
#define NS_CORE_COMMON_MESSAGE_H_

typedef void *message_t;
typedef struct _buffer buffer_t;
typedef struct _message_info message_info_t;
typedef struct _message_pool message_pool_t;

enum {
    MSG_INFO_LIST_ALL       = 0, // identifies the all messages list (maintained by the message_pool)
    MSG_INFO_LIST_INTERFACE = 1, // identifies the list for per-interface message queue
    MSG_INFO_NUM_LISTS      = 2, // number of list
};

struct _message_info {
    message_t *next[MSG_INFO_NUM_LISTS]; // a pointer to the next message in doubly linked list
    message_t *prev[MSG_INFO_NUM_LISTS]; // a pointer to the previous message ind doubly linked list
    message_pool_t *message_pool;
    uint16_t reserved;
    uint16_t length;
    uint16_t offset;
    uint8_t type;
    uint8_t sub_type;
    uint8_t priority;
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

struct _message_pool {
    uint16_t num_free_buffers;
    buffer_t buffers[MSG_NUM_BUFFERS];
    buffer_t *free_buffers;
    // --- message pool functions
    message_t (* new)(uint8_t type, uint16_t reserved, uint8_t priority);
    ns_error_t (* set_length)(message_t *message, uint16_t length);
    uint16_t (* get_length)(message_t *message);
    int (* write)(message_t *message, uint16_t offset, uint16_t length, void *buf);
    int (* read)(message_t *message, uint16_t offset, uint16_t length, void *buf);
    void (* free)(message_t *message);
};

void message_pool_make_new(void *instance);

#endif // NS_CORE_COMMON_MESSAGE_H_
