#ifndef NS_CORE_NET_DNS_HEADERS_H
#define NS_CORE_NET_DNS_HEADERS_H

#include "core/core-config.h"
#include "ns/ip6.h"
#include "core/common/encoding.h"
#include "core/common/message.h"
#include <string.h>

typedef enum _dns_header_type {
    DNS_HEADER_TYPE_QUERY    = 0,
    DNS_HEADER_TYPE_RESPONSE = 1,
} dns_header_type_t;

typedef enum _dns_header_query_type {
    DNS_HEADER_QUERY_TYPE_STANDARD = 0,
    DNS_HEADER_QUERY_TYPE_INVERSE  = 1,
    DNS_HEADER_QUERY_TYPE_STATUS   = 2,
    DNS_HEADER_QUERY_TYPE_NOTIFY   = 4,
    DNS_HEADER_QUERY_TYPE_UPDATE   = 5,
} dns_header_query_type_t;

typedef enum _dns_header_response {
    DNS_HEADER_RESPONSE_SUCCESS         = 0,
    DNS_HEADER_RESPONSE_FORMAT_ERROR    = 1,
    DNS_HEADER_RESPONSE_SERVER_FAILURE  = 2,
    DNS_HEADER_RESPONSE_NAME_ERROR      = 3,
    DNS_HEADER_RESPONSE_NOT_IMPLEMENTED = 4,
    DNS_HEADER_RESPONSE_REFUSED         = 5,
    DNS_HEADER_RESPONSE_NOT_AUTH        = 9,
    DNS_HEADER_RESPONSE_NOT_ZONE        = 10,
    DNS_HEADER_RESPONSE_BAD_NAME        = 20,
    DNS_HEADER_RESPONSE_BAD_ALG         = 21,
    DNS_HEADER_RESPONSE_BAD_TRUNCATION  = 22,
} dns_header_response_t;

enum {
    DNS_HEADER_QR_FLAG_OFFSET = 7,
    DNS_HEADER_QR_FLAG_MASK   = 0x01 << DNS_HEADER_QR_FLAG_OFFSET,
    DNS_HEADER_OP_CODE_OFFSET = 3,
    DNS_HEADER_OP_CODE_MASK   = 0x0f << DNS_HEADER_OP_CODE_OFFSET,
    DNS_HEADER_AA_FLAG_OFFSET = 2,
    DNS_HEADER_AA_FLAG_MASK   = 0x01 << DNS_HEADER_AA_FLAG_OFFSET,
    DNS_HEADER_TC_FLAG_OFFSET = 1,
    DNS_HEADER_TC_FLAG_MASK   = 0x01 << DNS_HEADER_TC_FLAG_OFFSET,
    DNS_HEADER_RD_FLAG_OFFSET = 0,
    DNS_HEADER_RD_FLAG_MASK   = 0x01 << DNS_HEADER_RD_FLAG_OFFSET,

    DNS_HEADER_RA_FLAG_OFFSET = 7,
    DNS_HEADER_RA_FLAG_MASK   = 0x01 << DNS_HEADER_RA_FLAG_OFFSET,
    DNS_HEADER_R_CODE_OFFSET  = 0,
    DNS_HEADER_R_CODE_MASK    = 0x0f << DNS_HEADER_R_CODE_OFFSET,
};

typedef struct _dns_header {
    uint16_t message_id;
    uint8_t flags[2];
    uint16_t qd_count;
    uint16_t an_count;
    uint16_t ns_count;
    uint16_t ar_count;
} dns_header_t;

typedef struct _dns_resource_record {
    uint16_t type;
    uint16_t mclass;
    uint32_t ttl;
    uint16_t length;
} dns_resource_record_t;

enum {
    DNS_RESOURCE_RECORD_AAAA_TYPE   = 0x1c,
    DNS_RESOURCE_RECORD_AAAA_CLASS  = 0x01,
    DNS_RESOURCE_RECORD_AAAA_LENGTH = 16,
};

typedef struct _dns_resource_record_aaaa {
    dns_resource_record_t resource_record;
    ns_ip6_addr_t addr;
} dns_resource_record_aaaa_t;

typedef struct _dns_question {
    uint16_t type;
    uint16_t mclass;
} dns_question_t;

enum {
    DNS_QUESTION_AAAA_TYPE  = 0x1c,
    DNS_QUESTION_AAAA_CLASS = 0x01,
};

typedef dns_question_t dns_question_aaaa_t;

// --- dns header functions
void
dns_header_ctor(dns_header_t *dns_header);

uint16_t
dns_header_get_message_id(dns_header_t *dns_header);

void
dns_header_set_message_id(dns_header_t *dns_header, uint16_t message_id);

dns_header_type_t
dns_header_get_type(dns_header_t *dns_header);

void
dns_header_set_type(dns_header_t *dns_header, dns_header_type_t type);

dns_header_query_type_t
dns_header_get_query_type(dns_header_t *dns_header);

void
dns_header_set_query_type(dns_header_t *dns_header, dns_header_query_type_t type);

bool
dns_header_is_authoritative_answer_flag_set(dns_header_t *dns_header);

void
dns_header_clear_authoritative_answer_flag(dns_header_t *dns_header);

void
dns_header_set_authoritative_answer_flag(dns_header_t *dns_header);

bool
dns_header_is_truncation_flag_set(dns_header_t *dns_header);

void
dns_header_clear_truncation_flag(dns_header_t *dns_header);

void
dns_header_set_truncation_flag(dns_header_t *dns_header);

bool
dns_header_is_recursion_desired_flag_set(dns_header_t *dns_header);

void
dns_header_clear_recursion_desired_flag(dns_header_t *dns_header);

void
dns_header_set_recursion_desired_flag(dns_header_t *dns_header);

bool
dns_header_is_recursion_available_flag_set(dns_header_t *dns_header);

void
dns_header_clear_recursion_available_flag(dns_header_t *dns_header);

void
dns_header_set_recursion_available_flag(dns_header_t *dns_header);

dns_header_response_t
dns_header_get_response_code(dns_header_t *dns_header);

void
dns_header_set_response_code(dns_header_t *dns_header, dns_header_response_t response);

uint16_t
dns_header_get_question_count(dns_header_t *dns_header);

void
dns_header_set_question_count(dns_header_t *dns_header, uint16_t count);

uint16_t
dns_header_get_answer_count(dns_header_t *dns_header);

void
dns_header_set_answer_count(dns_header_t *dns_header, uint16_t count);

uint16_t
dns_header_get_authority_records_count(dns_header_t *dns_header);

void
dns_header_set_authority_records_count(dns_header_t *dns_header, uint16_t count);

uint16_t
dns_header_get_additional_records_count(dns_header_t *dns_header);

void
dns_header_set_additional_records_count(dns_header_t *dns_header, uint16_t count);

// --- dns resource record functions
uint16_t
dns_resource_record_get_type(dns_resource_record_t *dns_resource_record);

void
dns_resource_record_set_type(dns_resource_record_t *dns_resource_record, uint16_t type);

uint16_t
dns_resource_record_get_class(dns_resource_record_t *dns_resource_record);

void
dns_resource_record_set_class(dns_resource_record_t *dns_resource_record, uint16_t aclass);

uint32_t
dns_resource_record_get_ttl(dns_resource_record_t *dns_resource_record);

void
dns_resource_record_set_ttl(dns_resource_record_t *dns_resource_record, uint32_t ttl);

uint16_t
dns_resource_record_get_length(dns_resource_record_t *dns_resource_record);

void
dns_resource_record_set_length(dns_resource_record_t *dns_resource_record, uint16_t length);

// --- dns resource record aaaa
void
dns_resource_record_aaaa_init(dns_resource_record_aaaa_t *dns_resource_record_aaaa);

void
dns_resource_record_aaaa_set_addr(dns_resource_record_aaaa_t *dns_resource_record_aaaa, ns_ip6_addr_t addr);

ns_ip6_addr_t
dns_resource_record_aaaa_get_addr(dns_resource_record_aaaa_t *dns_resource_record_aaaa);

// --- dns question
void
dns_question_ctor(dns_question_t *dns_question, uint16_t type, uint16_t aclass);

uint16_t
dns_question_get_type(dns_question_t *dns_question);

void
dns_question_set_type(dns_question_t *dns_question, uint16_t type);

uint16_t
dns_question_get_class(dns_question_t *dns_question);

void
dns_question_set_class(dns_question_t *dns_question, uint16_t aclass);

// --- dns question aaaa
void
dns_question_aaaa_ctor(dns_question_aaaa_t *dns_question_aaaa);

ns_error_t
dns_question_aaaa_append_to(dns_question_aaaa_t *dns_question_aaaa, message_t message);

#endif // NS_CORE_NET_DNS_HEADERS_H
