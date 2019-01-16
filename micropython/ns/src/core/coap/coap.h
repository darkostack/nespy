#ifndef NS_CORE_COAP_COAP_H_
#define NS_CORE_COAP_COAP_H_

#include "core/core-config.h"
#include "ns/coap.h"
#include "core/coap/coap_header.h"
#include "core/common/debug.h"
#include "core/common/message.h"
#include "core/common/timer.h"
#include "core/net/ip6.h"
#include "core/net/netif.h"
#include "core/net/udp6.h"

// coap protocol constants (RFC 7252)
enum {
    COAP_ACK_TIMEOUT                   = NS_CONFIG_COAP_ACK_TIMEOUT,
    COAP_ACK_RANDOM_FACTOR_NUMERATOR   = NS_CONFIG_COAP_ACK_RANDOM_FACTOR_NUMERATOR,
    COAP_ACK_RANDOM_FACTOR_DENOMINATOR = NS_CONFIG_COAP_ACK_RANDOM_FACTOR_DENOMINATOR,
    COAP_MAX_RETRANSMIT                = NS_CONFIG_COAP_MAX_RETRANSMIT,
    COAP_NSTART                        = 1,
    COAP_DEFAULT_LEISURE               = 5,
    COAP_PROBING_RATE                  = 1,

    // Note that 2 << (COAP_MAX_RETRANSMIT -1 ) is equal to COAP_MAX_RETRANSMIT power of 2
    COAP_MAX_TRANSMIT_SPAN =
        COAP_ACK_TIMEOUT * ((2 << (COAP_MAX_RETRANSMIT - 1)) - 1) * COAP_ACK_RANDOM_FACTOR_NUMERATOR / COAP_ACK_RANDOM_FACTOR_DENOMINATOR,
    COAP_MAX_TRANSMIT_WAIT =
        COAP_ACK_TIMEOUT * ((2 << COAP_MAX_RETRANSMIT) - 1) * COAP_ACK_RANDOM_FACTOR_NUMERATOR / COAP_ACK_RANDOM_FACTOR_DENOMINATOR,
    COAP_MAX_LATENCY       = 100,
    COAP_PROCESSING_DELAY  = COAP_ACK_TIMEOUT,
    COAP_MAX_RTT           = 2 * COAP_MAX_LATENCY + COAP_PROCESSING_DELAY,
    COAP_EXCHANGE_LIFETIME = COAP_MAX_TRANSMIT_SPAN + 2 * (COAP_MAX_LATENCY) + COAP_PROCESSING_DELAY,
    COAP_NON_LIFETIME      = COAP_MAX_TRANSMIT_SPAN + COAP_MAX_LATENCY
};

typedef struct _coap_metadata {
    ip6_addr_t source_addr;
    ip6_addr_t destination_addr;
    uint16_t destination_port;
    ns_coap_response_handler_func_t response_handler;
    void *response_context;
    uint32_t next_timer_shot;
    uint32_t retransmission_timeout;
    uint8_t retransmission_count;
    bool acknowledged : 1;
    bool confirmable : 1;
} coap_metadata_t;

enum {
    COAP_RESOURCE_MAX_RECEIVE_URI_PATH = 32, // maximum supported URI path on received message
};

typedef ns_coap_resource_t coap_resource_t;

typedef struct _coap_enqueued_response_header {
    uint32_t dequeue_time;
    const ip6_message_info_t message_info;
} coap_enqueued_response_header_t;

typedef struct _coap_response_queue {
    message_queue_t queue;
    timer_milli_t timer;
} coap_response_queue_t;

typedef ns_error_t (*coap_base_interceptor_func_t)(const message_t message,
                                                   const ip6_message_info_t *message_info,
                                                   void *context);
typedef struct _coap_base {
    void *instance;
    message_queue_t pending_requests;
    uint16_t message_id;
    timer_milli_t retransmission_timer;

    coap_resource_t *resources;

    void *context;
    coap_base_interceptor_func_t interceptor;
    coap_response_queue_t responses_queue;

    ns_coap_request_handler_func_t default_handler;
    void *default_handler_context;
} coap_base_t;

typedef struct _coap {
    coap_base_t base;
} coap_t;

typedef struct _coap_application_coap {
    coap_base_t base;
} coap_application_coap_t;

// --- coap metadata functions
void
coap_metadata_ctor(coap_metadata_t *coap_metadata);

void
coap_metadata_ctor2(coap_metadata_t *coap_metadata,
                    bool confirmable
                    const ip6_message_info_t *message_info,
                    ns_coap_request_handler_func_t handler,
                    void *context);

ns_error_t
coap_metadata_append_to(coap_metadata_t *coap_metadata, message_t message);

uint16_t
coap_metadata_read_from(coap_metadata_t *coap_metadata, const message_t message);

int
coap_metadata_update_in(coap_metadata_t *coap_metadata, message_t message);

bool
coap_metadata_is_earlier(coap_metadata_t *coap_metadata, uint32_t time);

bool
coap_metadata_is_later(coap_metadata_t *coap_metadata, uint32_t time);

// --- coap resource functions
void
coap_resource_ctor(coap_resource_t *coap_resource, ns_coap_request_handler_func_t handler, void *context);

coap_resource_t *
coap_resource_get_next(coap_resource_t *coap_resource);

const char *
coap_resource_get_uri_path(coap_resource_t *coap_resource);

// --- coap enqueued response header functions
void
coap_enqueued_response_header_ctor(coap_enqueued_response_header_t *coap_enqueued_response_header);

void
coap_enqueued_response_header_ctor2(coap_enqueued_response_header_t *coap_enqueued_response_header,
                                    ip6_message_info_t *message_info);

ns_error_t
coap_enqueued_response_header_append_to(coap_enqueued_response_header_t *coap_enqueued_response_header,
                                        message_t message);

uint16_t
coap_enqueued_response_header_read_from(coap_enqueued_response_header_t *coap_enqueued_response_header,
                                        const message_t message);

void
coap_enqueued_response_header_remove_from(coap_enqueued_response_header_t *coap_enqueued_response_header,
                                          message_t message);

bool
coap_enqueued_response_header_is_earlier(coap_enqueued_response_header_t *coap_enqueued_response_header,
                                         uint32_t time);

const ip6_message_info_t *
coap_enqueued_response_header_get_message_info(coap_enqueued_response_header_t *coap_enqueued_response_header);

// --- coap response queue functions
void
coap_response_queue_ctor(coap_response_queue_t *coap_response_queue,
                         void *instance, 
                         timer_handler_func_t handler,
                         void *context);

void
coap_response_queue_enqueue_response(coap_response_queue_t *coap_response_queue,
                                     message_t message,
                                     const ip6_message_info_t *message_info);

void
coap_response_queue_dequeue_oldest_response(coap_response_queue_t *coap_response_queue);

void
coap_response_queue_dequeue_all_responses(coap_response_queue_t *coap_response_queue);

ns_error_t
coap_response_queue_get_matched_response_copy(coap_response_queue_t *coap_response_queue,
                                              const coap_header_t *header,
                                              const ip6_message_info_t *message_info,
                                              message_t response);

#endif // NS_CORE_COAP_COAP_H_
