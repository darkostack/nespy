#ifndef NS_COAP_H_
#define NS_COAP_H_

#include "ns/ip6.h"
#include "ns/message.h"
#include <stdint.h>

#define NS_DEFAULT_COAP_PORT 5683 // default CoAP ports, as specified in (RFC 7252)
#define NS_COAP_MAX_TOKEN_LENGTH 8 // max token length as specified in (RFC 7252)

typedef enum _ns_coap_type {
    NS_COAP_TYPE_CONFIRMABLE     = 0x00,
    NS_COAP_TYPE_NON_CONFIRMABLE = 0x10,
    NS_COAP_TYPE_ACKNOWLEDGMENT  = 0x20,
    NS_COAP_TYPE_RESET           = 0x30,
} ns_coap_type_t;

// helper macro to define CoAP code values
#define NS_COAP_CODE(c, d) ((((c)&0x7) << 5) | ((d)&0x1f))

typedef enum _ns_coap_code {
    NS_COAP_CODE_EMPTY  = NS_COAP_CODE(0, 0),
    NS_COAP_CODE_GET    = NS_COAP_CODE(0, 1),
    NS_COAP_CODE_POST   = NS_COAP_CODE(0, 2),
    NS_COAP_CODE_PUT    = NS_COAP_CODE(0, 3),
    NS_COAP_CODE_DELETE = NS_COAP_CODE(0, 4),

    NS_COAP_CODE_RESPONSE_MIN = NS_COAP_CODE(2, 0),
    NS_COAP_CODE_CREATED      = NS_COAP_CODE(2, 1),
    NS_COAP_CODE_DELETED      = NS_COAP_CODE(2, 2),
    NS_COAP_CODE_VALID        = NS_COAP_CODE(2, 3),
    NS_COAP_CODE_CHANGED      = NS_COAP_CODE(2, 4),
    NS_COAP_CODE_CONTENT      = NS_COAP_CODE(2, 5),

    NS_COAP_CODE_BAD_REQUEST         = NS_COAP_CODE(4, 0),
    NS_COAP_CODE_UNAUTHORIZED        = NS_COAP_CODE(4, 1),
    NS_COAP_CODE_BAD_OPTION          = NS_COAP_CODE(4, 2),
    NS_COAP_CODE_FORBIDDEN           = NS_COAP_CODE(4, 3),
    NS_COAP_CODE_NOT_FOUND           = NS_COAP_CODE(4, 4),
    NS_COAP_CODE_METHOD_NOT_ALLOWED  = NS_COAP_CODE(4, 5),
    NS_COAP_CODE_NOT_ACCEPTABLE      = NS_COAP_CODE(4, 6),
    NS_COAP_CODE_PRECONDITION_FAILED = NS_COAP_CODE(4, 12),
    NS_COAP_CODE_REQUEST_TOO_LARGE   = NS_COAP_CODE(4, 13),
    NS_COAP_CODE_UNSUPPORTED_FORMAT  = NS_COAP_CODE(4, 15),

    NS_COAP_CODE_INTERNAL_ERROR      = NS_COAP_CODE(5, 0),
    NS_COAP_CODE_NOT_IMPLEMENTED     = NS_COAP_CODE(5, 1),
    NS_COAP_CODE_BAD_GATEWAY         = NS_COAP_CODE(5, 2),
    NS_COAP_CODE_SERVICE_UNAVAILABLE = NS_COAP_CODE(5, 3),
    NS_COAP_CODE_GATEWAY_TIMEOUT     = NS_COAP_CODE(5, 4),
    NS_COAP_CODE_PROXY_NOT_SUPPORTED = NS_COAP_CODE(5, 5),
} ns_coap_code_t;

typedef enum _ns_coap_option_type {
    NS_COAP_OPTION_IF_MATCH       = 1,
    NS_COAP_OPTION_URI_HOST       = 3,
    NS_COAP_OPTION_E_TAG          = 4,
    NS_COAP_OPTION_IF_NONE_MATCH  = 5,
    NS_COAP_OPTION_OBSERVE        = 6,
    NS_COAP_OPTION_URI_PORT       = 7,
    NS_COAP_OPTION_LOCATION_PATH  = 8,
    NS_COAP_OPTION_URI_PATH       = 11,
    NS_COAP_OPTION_CONTENT_FORMAT = 12,
    NS_COAP_OPTION_MAX_AGE        = 14,
    NS_COAP_OPTION_URI_QUERY      = 15,
    NS_COAP_OPTION_ACCEPT         = 17,
    NS_COAP_OPTION_LOCATION_QUERY = 20,
    NS_COAP_OPTION_PROXY_URI      = 35,
    NS_COAP_OPTION_PROXY_SCHEME   = 39,
    NS_COAP_OPTION_SIZE1          = 60,
} ns_coap_option_type_t;

typedef struct _ns_coap_option {
    uint16_t number;
    uint16_t length;
    const uint8_t *value;
} ns_coap_option_t;

typedef enum _ns_coap_option_content_format {
    NS_COAP_OPTION_CONTENT_FORMAT_TEXT_PLAIN   = 0,  // text/plain
    NS_COAP_OPTION_CONTENT_FORMAT_LINK_FORMAT  = 40, // application/link-format
    NS_COAP_OPTION_CONTENT_FORMAT_XML          = 41, // application/xml
    NS_COAP_OPTION_CONTENT_FORMAT_OCTET_STREAM = 42, // application/octet-stream
    NS_COAP_OPTION_CONTENT_FORMAT_EXI          = 47, // application/exi
    NS_COAP_OPTION_CONTENT_FORMAT_JSON         = 50, // application/json
    NS_COAP_OPTION_CONTENT_FORMAT_PKCS10       = 70, // application/pkcs10
    NS_COAP_OPTION_CONTENT_FORMAT_PKCS7        = 80, // application/pkcs7
    NS_COAP_OPTION_CONTENT_FORMAT_JWS          = 101 // application/json-web-signature
} ns_coap_option_content_format_t;

#define NS_COAP_HEADER_MAX_LENGTH 512 // max CoAP header length (bytes)

typedef struct _ns_coap_header {
    union {
        struct {
            uint8_t version_type_token;
            uint8_t code;
            uint16_t message_id;
        } fields;
        uint8_t bytes[NS_COAP_HEADER_MAX_LENGTH];
    } header;
    uint16_t header_length;
    uint16_t option_last;
    uint16_t first_option_offset;
    uint16_t next_option_offset;
    ns_coap_option_t option;
} ns_coap_header_t;

// this function poiner is called when a CoAP response is received or on the request timeout
typedef void (*ns_coap_response_handler_func_t)(void *context,
                                                ns_coap_header_t *header,
                                                ns_message_t message,
                                                const ns_message_info_t *message_info,
                                                ns_error_t result);

// this function pointer is called when a CoAP request with given URI-path is received
typedef void (*ns_coap_request_handler_func_t)(void *context,
                                               ns_coap_header_t *header,
                                               ns_message_t message,
                                               const ns_message_info_t *message_info);

typedef struct _ns_coap_resource ns_coap_resource_t;
struct _ns_coap_resource {
    const char *uri_path;
    ns_coap_request_handler_func_t handler;
    void *context;
    ns_coap_resource_t *next;
};

void
ns_coap_header_init(ns_coap_header_t *header, ns_coap_type_t type, ns_coap_code_t code);

void
ns_coap_header_set_token(ns_coap_header_t *header, const uint8_t *token, uint8_t token_length);

void
ns_coap_header_generate_token(ns_coap_header_t *header, uint8_t token_length);

ns_error_t
ns_coap_header_append_content_format_option(ns_coap_header_t *header, ns_coap_option_content_format_t content_format);

ns_error_t
ns_coap_header_append_option(ns_coap_header_t *header, const ns_coap_option_t *option);

ns_error_t
ns_coap_header_append_uint_option(ns_coap_header_t *header, uint16_t number, uint32_t value);

ns_error_t
ns_coap_header_append_observe_option(ns_coap_header_t *header, uint32_t observe);

ns_error_t
ns_coap_header_append_uri_path_options(ns_coap_header_t *header, const char *uri_path);

ns_error_t
ns_coap_header_append_proxy_uri_option(ns_coap_header_t *header, const char *uri_path);

ns_error_t
ns_coap_header_append_max_age_option(ns_coap_header_t *header, uint32_t max_age);

ns_error_t
ns_coap_header_append_uri_query_option(ns_coap_header_t *header, const char *uri_query);

ns_error_t
ns_coap_header_set_payload_marker(ns_coap_header_t *header);

void
ns_coap_header_set_message_id(ns_coap_header_t *header, uint16_t message_id);

ns_coap_type_t
ns_coap_header_get_type(const ns_coap_header_t *header);

ns_coap_code_t
ns_coap_header_get_code(const ns_coap_header_t *header);

const char *
ns_coap_header_code_to_string(const ns_coap_header_t *header);

uint16_t
ns_coap_header_get_message_id(const ns_coap_header_t *header);

uint8_t
ns_coap_header_get_token_length(const ns_coap_header_t *header);

const uint8_t *
ns_coap_header_get_token(const ns_coap_header_t *header);

const ns_coap_option_t *
ns_coap_header_get_first_option(ns_coap_header_t *header);

const ns_coap_option_t *
ns_coap_header_get_next_option(ns_coap_header_t *header);

ns_message_t
ns_coap_new_message(ns_instance_t instance, const ns_coap_header_t *header, const ns_message_settings_t *settings);

ns_error_t
ns_coap_send_request(ns_instance_t instance,
                     ns_message_t message,
                     const ns_message_info_t *message_info,
                     ns_coap_response_handler_func_t handler,
                     void *context);

ns_error_t
ns_coap_start(ns_instance_t instance, uint16_t port);

ns_error_t
ns_coap_stop(ns_instance_t instance);

ns_error_t
ns_coap_add_resource(ns_instance_t instance, ns_coap_resource_t *resource);

void
ns_coap_remove_resource(ns_instance_t instance, ns_coap_resource_t *resource);

void
ns_coap_set_default_handler(ns_instance_t instance, ns_coap_request_handler_func_t handler, void *context);

ns_error_t
ns_coap_send_response(ns_instance_t instance, ns_message_t message, const ns_message_info_t *message_info);

#endif // NS_COAP_H_
