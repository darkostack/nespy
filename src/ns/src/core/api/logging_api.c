#include "core/core-config.h"
#include "ns/logging.h"
#include "core/common/instance.h"

ns_log_level_t
ns_logging_get_level(void)
{
    return instance_get_log_level();
}

ns_error_t
ns_logging_set_level(ns_log_level_t log_level)
{
    ns_error_t error = NS_ERROR_DISABLED_FEATURE;
#if NS_CONFIG_ENABLE_DYNAMIC_LOG_LEVEL
    instance_set_log_level(log_level);
    error = NS_ERROR_NONE;
#endif
    return error;
}
