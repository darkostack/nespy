#ifndef NS_CORE_CONFIG_H_
#define NS_CORE_CONFIG_H_

#include "ns/include/config.h"

#define NS_CORE_CONFIG_H_IN
#ifdef NS_PROJECT_CORE_CONFIG_FILE
#include NS_PROJECT_CORE_CONFIG_FILE
#endif
#include "ns/core/core-default-config.h"
#undef NS_CORE_CONFIG_H_IN

#endif // NS_CORE_CONFIG_H_
