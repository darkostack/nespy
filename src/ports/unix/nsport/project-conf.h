#ifndef PROJECT_CONF_H_
#define PROJECT_CONF_H_

// CSMA config
#define CSMA_CONF_SEND_SOFT_ACK 1

// TCP config
#define UIP_CONF_TCP 1

// LOG config
#define LOG_CONF_LEVEL_RADIO    LOG_LEVEL_NONE
#define LOG_CONF_LEVEL_MAC      LOG_LEVEL_NONE
#define LOG_CONF_LEVEL_FRAMER   LOG_LEVEL_NONE
#define LOG_CONF_LEVEL_COAP     LOG_LEVEL_NONE
#define LOG_LEVEL_APP           LOG_LEVEL_DBG

// Application config
#ifdef APP_CONF_WITH_COAP
// enable client-side support for COAP observe
#define COAP_OBSERVE_CLIENT     1
#endif

#endif // PROJECT_CONF_H_
