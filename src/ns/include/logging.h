#ifndef NS_LOGGING_H_
#define NS_LOGGING_H_

#include "ns/include/error.h"
#include "ns/include/platform/logging.h"

ns_log_level_t
ns_logging_get_level(void);

ns_error_t
ns_logging_set_level(ns_log_level_t log_level);

#endif // NS_LOGGING_H_
