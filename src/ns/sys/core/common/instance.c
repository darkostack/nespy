#include "ns/sys/core/common/instance.h"

static NS_DEFINE_ALIGNED_VAR(instance_raw, sizeof(instance_t), uint64_t);

// --- extern instance objects constructor functions
extern void
timer_scheduler_ctor(void *instance);

extern void
tasklet_scheduler_ctor(void *instance);

extern void
message_pool_ctor(void *instance);

extern void
heap_ctor(void *instance);

// --- instance functions
instance_t *
instance_init(void)
{
    instance_t *inst = instance_get();

    VERIFY_OR_EXIT(inst->is_initialized == false);

    timer_scheduler_ctor((void *)inst);
    tasklet_scheduler_ctor((void *)inst);
    message_pool_ctor((void *)inst);
    heap_ctor((void *)inst);

    inst->is_initialized = true;

    extern ns_error_t test_message_write_read(void);
    test_message_write_read();

    extern ns_error_t test_message_utility(void);
    test_message_utility();

    extern ns_error_t test_message_queue(void);
    test_message_queue();

    extern ns_error_t test_message_priority_queue(void);
    test_message_priority_queue();

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
instance_get_timer_scheduler(void)
{
    instance_t *inst = instance_get();
    return (timer_scheduler_t *)&inst->timer_sched;
}

tasklet_scheduler_t *
instance_get_tasklet_scheduler(void)
{
    instance_t *inst = instance_get();
    return (tasklet_scheduler_t *)&inst->tasklet_sched;
}

message_pool_t *
instance_get_message_pool(void)
{
    instance_t *inst = instance_get();
    return (message_pool_t *)&inst->message_pool;
}

heap_t *
instance_get_heap(void)
{
    instance_t *inst = instance_get();
    return (heap_t *)&inst->heap;
}
