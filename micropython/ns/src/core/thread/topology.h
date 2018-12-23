#ifndef NS_CORE_THREAD_TOPOLOGY_H_
#define NS_CORE_THREAD_TOPOLOGY_H_

#include "core/core-config.h"
#include "core/thread/link_quality.h"

typedef struct _neighbor neighbor_t;

typedef enum _neighbor_state {
    NEIGHBOR_STATE_INVALID,
    NEIGHBOR_STATE_RESTORED,
    NEIGHBOR_STATE_PARENT_REQUEST,
    NEIGHBOR_STATE_PARENT_RESPONSE,
    NEIGHBOR_STATE_CHILD_REQUEST,
    NEIGHBOR_STATE_LINK_REQUEST,
    NEIGHBOR_STATE_CHILD_UPDATE_REQUEST,
    NEIGHBOR_STATE_VALID,
} neighbor_state_t;

struct _neighbor {
    uint8_t state : 3;
    link_quality_info_t link_info;
};

#endif // NS_CORE_THREAD_TOPOLOGY_H_
