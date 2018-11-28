#include "ns/include/error.h"
#include "ns/sys/core/common/instance.h"

static NS_DEFINE_ALIGNED_VAR(instance_raw, sizeof(instance_t), uint64_t);

// --- extern instance objects constructor functions
extern void
timer_scheduler_ctor(timer_scheduler_t *timer_scheduler);

extern void
tasklet_scheduler_ctor(tasklet_scheduler_t *tasklet_scheduler);

extern void
message_pool_ctor(message_pool_t *message_pool);

extern void
heap_ctor(heap_t *heap);

// --- instance functions
instance_t *
instance_init(void)
{
    instance_t *inst = instance_get();

    VERIFY_OR_EXIT(inst->is_initialized == false);

    timer_scheduler_ctor((timer_scheduler_t *)&inst->timer_sched);
    tasklet_scheduler_ctor((tasklet_scheduler_t *)&inst->tasklet_sched);
    message_pool_ctor((message_pool_t *)&inst->message_pool);
    heap_ctor((heap_t *)&inst->heap);

    inst->is_initialized = true;

    extern ns_error_t test_message_write_read(void *instance);
    test_message_write_read((void *)inst);

    extern ns_error_t test_message_utility(void *instance);
    test_message_utility((void *)inst);

    extern ns_error_t test_message_queue(void *instance);
    test_message_queue((void *)inst);

    extern ns_error_t test_message_priority_queue(void *instance);
    test_message_priority_queue((void *)inst);

    extern ns_error_t test_heap_allocate_single(void *instance);
    test_heap_allocate_single((void *)inst);

    extern ns_error_t test_heap_allocate_multiple(void *instance);
    test_heap_allocate_multiple((void *)inst);

exit:
    return inst;
}

instance_t *
instance_get(void)
{
    void *inst = &instance_raw;
    return (instance_t *)inst;
}

timer_scheduler_t *
instance_get_timer_scheduler(instance_t *instance)
{
    return (timer_scheduler_t *)&instance->timer_sched;
}

tasklet_scheduler_t *
instance_get_tasklet_scheduler(instance_t *instance)
{
    return (tasklet_scheduler_t *)&instance->tasklet_sched;
}

message_pool_t *
instance_get_message_pool(instance_t *instance)
{
    return (message_pool_t *)&instance->message_pool;
}

heap_t *
instance_get_heap(instance_t *instance)
{
    return (heap_t *)&instance->heap;
}
