#include "ns/error.h"
#include "core/common/instance.h"
#include "core/crypto/mbedtls.h"

static NS_DEFINE_ALIGNED_VAR(s_instance_raw, sizeof(instance_t), uint64_t);

// --- instance functions
instance_t *
instance_ctor(void)
{
    instance_t *inst = instance_get();

    VERIFY_OR_EXIT(inst->is_initialized == false);

    //instance constructors
    timer_scheduler_ctor(&inst->timer_milli_scheduler);
#if NS_CONFIG_ENABLE_PLATFORM_USEC_TIMER
    timer_scheduler_ctor(&inst->timer_micro_scheduler);
#endif // NS_CONFIG_ENABLE_PLATFORM_USEC_TIMER
    tasklet_scheduler_ctor(&inst->tasklet_scheduler);
#if NS_CONFIG_ENABLE_DYNAMIC_LOG_LEVEL
    inst->log_level = (ns_log_level_t)NS_CONFIG_LOG_LEVEL;
#endif // NS_CONFIG_ENABLE_DYNAMIC_LOG_LEVEL
    notifier_ctor(inst, &inst->notifier);
    message_pool_ctor(&inst->message_pool);
#if NS_RADIO || NS_ENABLE_RAW_LINK_API
    mac_link_raw_ctor(inst, &inst->link_raw);
#endif // NS_RADIO || NS_ENABLE_RAW_LINK_API
    heap_ctor(&inst->heap);
    mbedtls_ctor();
    ip6_ctor(&inst->ip6, inst);

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

    extern ns_error_t test_string(void);
    test_string();

    extern ns_error_t test_mac_header(void);
    test_mac_header();

    extern ns_error_t test_mac_channel_mask(void);
    test_mac_channel_mask();

    extern ns_error_t test_link_quality_calculations(void);
    test_link_quality_calculations();

    extern ns_error_t test_rss_averager(void);
    test_rss_averager();

    extern void test_logging(void);
    test_logging();

    extern ns_error_t test_crypto_aes(void);
    test_crypto_aes();

    extern ns_error_t test_hmac_sha256(void);
    test_hmac_sha256();

    extern ns_error_t test_ip6_address(void);
    test_ip6_address();

    extern ns_error_t test_ip6_netif(void);
    test_ip6_netif();

    extern ns_error_t test_meshcop_panid_tlv(void);
    test_meshcop_panid_tlv();

exit:
    return inst;
}

instance_t *
instance_get(void)
{
    void *inst = &s_instance_raw;
    return (instance_t *)inst;
}

ns_log_level_t
instance_get_log_level(void)
{
#if NS_CONFIG_ENABLE_DYNAMIC_LOG_LEVEL
    instance_t *instance = instance_get();
    return instance->log_level;
#else
    return (ns_log_level_t)NS_CONFIG_LOG_LEVEL;
#endif
}

#if NS_CONFIG_ENABLE_DYNAMIC_LOG_LEVEL
void
instance_set_log_level(ns_log_level_t log_level)
{
    instance_t *instance = instance_get();
    instance->log_level = log_level;
}
#endif

notifier_t *
instance_get_notifier(void *instance)
{
    instance_t *inst = (instance_t *)instance;
    return &inst->notifier;
}

heap_t *
instance_get_heap(void *instance)
{
    instance_t *inst = (instance_t *)instance;
    return &inst->heap;
}

ip6_t *
instance_get_ip6(void *instance)
{
    instance_t *inst = (instance_t *)instance;
    return &inst->ip6;
}
