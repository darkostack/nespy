#include "core/net/dns_headers.h"

// --- dns header functions
void
dns_header_ctor(dns_header_t *dns_header)
{
    memset(dns_header, 0, sizeof(*dns_header));
}

uint16_t
dns_header_get_message_id(dns_header_t *dns_header)
{
    return encoding_big_endian_swap16(dns_header->message_id);
}

void
dns_header_set_message_id(dns_header_t *dns_header, uint16_t message_id)
{
    dns_header->message_id = encoding_big_endian_swap16(message_id);
}

dns_header_type_t
dns_header_get_type(dns_header_t *dns_header)
{
    return (dns_header_type_t)((dns_header->flags[0] & DNS_HEADER_QR_FLAG_MASK) >> DNS_HEADER_QR_FLAG_OFFSET);
}

void
dns_header_set_type(dns_header_t *dns_header, dns_header_type_t type)
{
    dns_header->flags[0] &= ~DNS_HEADER_QR_FLAG_MASK;
    dns_header->flags[0] |= (uint8_t)(type) << DNS_HEADER_QR_FLAG_OFFSET;
}

dns_header_query_type_t
dns_header_get_query_type(dns_header_t *dns_header)
{
    return (dns_header_query_type_t)((dns_header->flags[0] & DNS_HEADER_OP_CODE_MASK) >> DNS_HEADER_OP_CODE_OFFSET);
}

void
dns_header_set_query_type(dns_header_t *dns_header, dns_header_query_type_t type)
{
    dns_header->flags[0] &= ~DNS_HEADER_OP_CODE_MASK;
    dns_header->flags[0] |= (uint8_t)(type) << DNS_HEADER_OP_CODE_OFFSET;
}

bool
dns_header_is_authoritative_answer_flag_set(dns_header_t *dns_header)
{
    return (dns_header->flags[0] & DNS_HEADER_AA_FLAG_MASK) == DNS_HEADER_AA_FLAG_MASK;
}

void
dns_header_clear_authoritative_answer_flag(dns_header_t *dns_header)
{
    dns_header->flags[0] &= ~DNS_HEADER_AA_FLAG_MASK;
}

void
dns_header_set_authoritative_answer_flag(dns_header_t *dns_header)
{
    dns_header->flags[0] |= DNS_HEADER_AA_FLAG_MASK;
}

bool
dns_header_is_truncation_flag_set(dns_header_t *dns_header)
{
    return (dns_header->flags[0] & DNS_HEADER_TC_FLAG_MASK) == DNS_HEADER_TC_FLAG_MASK;
}

void
dns_header_clear_truncation_flag(dns_header_t *dns_header)
{
    dns_header->flags[0] &= ~DNS_HEADER_TC_FLAG_MASK;
}

void
dns_header_set_truncation_flag(dns_header_t *dns_header)
{
    dns_header->flags[0] |= DNS_HEADER_TC_FLAG_MASK;
}

bool
dns_header_is_recursion_desired_flag_set(dns_header_t *dns_header)
{
    return (dns_header->flags[0] & DNS_HEADER_RD_FLAG_MASK) == DNS_HEADER_RD_FLAG_MASK;
}

void
dns_header_clear_recursion_desired_flag(dns_header_t *dns_header)
{
    dns_header->flags[0] &= ~DNS_HEADER_RD_FLAG_MASK;
}

void
dns_header_set_recursion_desired_flag(dns_header_t *dns_header)
{
    dns_header->flags[0] |= DNS_HEADER_RD_FLAG_MASK;
}

bool
dns_header_is_recursion_available_flag_set(dns_header_t *dns_header)
{
    return (dns_header->flags[1] & DNS_HEADER_RA_FLAG_MASK) == DNS_HEADER_RA_FLAG_MASK;
}

void
dns_header_clear_recursion_available_flag(dns_header_t *dns_header)
{
    dns_header->flags[1] &= ~DNS_HEADER_RA_FLAG_MASK;
}

void
dns_header_set_recursion_available_flag(dns_header_t *dns_header)
{
    dns_header->flags[1] |= DNS_HEADER_RA_FLAG_MASK;
}

dns_header_response_t
dns_header_get_response_code(dns_header_t *dns_header)
{
    return (dns_header_response_t)((dns_header->flags[1] & DNS_HEADER_R_CODE_MASK) >> DNS_HEADER_R_CODE_OFFSET);
}

void
dns_header_set_response_code(dns_header_t *dns_header, dns_header_response_t response)
{
    dns_header->flags[1] &= ~DNS_HEADER_R_CODE_MASK;
    dns_header->flags[1] |= (uint8_t)(response) << DNS_HEADER_R_CODE_OFFSET;
}

uint16_t
dns_header_get_question_count(dns_header_t *dns_header)
{
    return encoding_big_endian_swap16(dns_header->qd_count);
}

void
dns_header_set_question_count(dns_header_t *dns_header, uint16_t count)
{
    dns_header->qd_count = encoding_big_endian_swap16(count);
}

uint16_t
dns_header_get_answer_count(dns_header_t *dns_header)
{
    return encoding_big_endian_swap16(dns_header->an_count);
}

void
dns_header_set_answer_count(dns_header_t *dns_header, uint16_t count)
{
    dns_header->an_count = encoding_big_endian_swap16(count);
}

uint16_t
dns_header_get_authority_records_count(dns_header_t *dns_header)
{
    return encoding_big_endian_swap16(dns_header->ns_count);
}

void
dns_header_set_authority_records_count(dns_header_t *dns_header, uint16_t count)
{
    dns_header->ns_count = encoding_big_endian_swap16(count);
}

uint16_t
dns_header_get_additional_records_count(dns_header_t *dns_header)
{
    return encoding_big_endian_swap16(dns_header->ar_count);
}

void
dns_header_set_additional_records_count(dns_header_t *dns_header, uint16_t count)
{
    dns_header->ar_count = encoding_big_endian_swap16(count);
}

// --- dns resource record functions
uint16_t
dns_resource_record_get_type(dns_resource_record_t *dns_resource_record)
{
    return encoding_big_endian_swap16(dns_resource_record->type);
}

void
dns_resource_record_set_type(dns_resource_record_t *dns_resource_record, uint16_t type)
{
    dns_resource_record->type = encoding_big_endian_swap16(type);
}

uint16_t
dns_resource_record_get_class(dns_resource_record_t *dns_resource_record)
{
    return encoding_big_endian_swap16(dns_resource_record->mclass);
}

void
dns_resource_record_set_class(dns_resource_record_t *dns_resource_record, uint16_t aclass)
{
    dns_resource_record->mclass = encoding_big_endian_swap16(aclass);
}

uint32_t
dns_resource_record_get_ttl(dns_resource_record_t *dns_resource_record)
{
    return encoding_big_endian_swap32(dns_resource_record->ttl);
}

void
dns_resource_record_set_ttl(dns_resource_record_t *dns_resource_record, uint32_t ttl)
{
    dns_resource_record->ttl = encoding_big_endian_swap32(ttl);
}

uint16_t
dns_resource_record_get_length(dns_resource_record_t *dns_resource_record)
{
    return encoding_big_endian_swap16(dns_resource_record->length);
}

void
dns_resource_record_set_length(dns_resource_record_t *dns_resource_record, uint16_t length)
{
    dns_resource_record->length = encoding_big_endian_swap16(length);
}

// --- dns resource record aaaa
void
dns_resource_record_aaaa_init(dns_resource_record_aaaa_t *dns_resource_record_aaaa)
{
    dns_resource_record_set_type(&dns_resource_record_aaaa->resource_record, DNS_RESOURCE_RECORD_AAAA_TYPE);
    dns_resource_record_set_class(&dns_resource_record_aaaa->resource_record, DNS_RESOURCE_RECORD_AAAA_CLASS);
    dns_resource_record_set_ttl(&dns_resource_record_aaaa->resource_record, 0);
    dns_resource_record_set_length(&dns_resource_record_aaaa->resource_record, DNS_RESOURCE_RECORD_AAAA_LENGTH);
    memset(&dns_resource_record_aaaa->addr, 0, sizeof(dns_resource_record_aaaa->addr));
}

void
dns_resource_record_aaaa_set_addr(dns_resource_record_aaaa_t *dns_resource_record_aaaa, ns_ip6_addr_t addr)
{
    dns_resource_record_aaaa->addr = addr;
}

ns_ip6_addr_t
dns_resource_record_aaaa_get_addr(dns_resource_record_aaaa_t *dns_resource_record_aaaa)
{
    return dns_resource_record_aaaa->addr;
}

// --- dns question
void
dns_question_ctor(dns_question_t *dns_question, uint16_t type, uint16_t aclass)
{
    dns_question_set_type(dns_question, type);
    dns_question_set_class(dns_question, aclass);
}

uint16_t
dns_question_get_type(dns_question_t *dns_question)
{
    return encoding_big_endian_swap16(dns_question->type);
}

void
dns_question_set_type(dns_question_t *dns_question, uint16_t type)
{
    dns_question->type = encoding_big_endian_swap16(type);
}

uint16_t
dns_question_get_class(dns_question_t *dns_question)
{
    return encoding_big_endian_swap16(dns_question->mclass);
}

void
dns_question_set_class(dns_question_t *dns_question, uint16_t aclass)
{
    dns_question->mclass = encoding_big_endian_swap16(aclass);
}

// --- dns question aaaa
void
dns_question_aaaa_ctor(dns_question_aaaa_t *dns_question_aaaa)
{
    dns_question_ctor((dns_question_t *)dns_question_aaaa, DNS_QUESTION_AAAA_TYPE, DNS_QUESTION_AAAA_CLASS);
}

ns_error_t
dns_question_aaaa_append_to(dns_question_aaaa_t *dns_question_aaaa, message_t message)
{
    return message_append(message, dns_question_aaaa, sizeof(*dns_question_aaaa)); 
}
