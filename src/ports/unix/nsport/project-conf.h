#ifndef PROJECT_CONF_H_
#define PROJECT_CONF_H_

// CSMA config
#define CSMA_CONF_SEND_SOFT_ACK 1

// TCP config
#define UIP_CONF_TCP 1

// LOG config
#define NESPY_LOG_DBG 0
#if NESPY_LOG_DBG
#define LOG_CONF_LEVEL_RADIO    LOG_LEVEL_DBG
#define LOG_CONF_LEVEL_MAC      LOG_LEVEL_DBG
#define LOG_CONF_LEVEL_FRAMER   LOG_LEVEL_DBG
#else
#define LOG_CONF_LEVEL_RADIO    LOG_LEVEL_NONE
#define LOG_CONF_LEVEL_MAC      LOG_LEVEL_NONE
#define LOG_CONF_LEVEL_FRAMER   LOG_LEVEL_NONE
#endif

// Application config
#ifdef APP_CONF_WITH_COAP
#define LOG_LEVEL_APP           LOG_LEVEL_DBG
// enable client-side support for COAP observe
#define COAP_OBSERVE_CLIENT     1
#endif

#endif // PROJECT_CONF_H_
