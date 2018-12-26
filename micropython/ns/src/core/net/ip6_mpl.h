#ifndef NS_CORE_NET_IP6_MPL_H_
#define NS_CORE_NET_IP6_MPL_H_

#include "core/core-config.h"
#include "core/common/message.h"
#include "core/common/timer.h"
#include "core/net/ip6_headers.h"

enum {
    IP6_OPTION_MPL_TYPE = 0x6d,
    IP6_OPTION_MPL_MIN_LENGTH = 2,
};

typedef enum _ip6_option_mpl_seed_id_length {
    IP6_OPTION_MPL_SEED_ID_LENGTH_0  = 0 << 6,
    IP6_OPTION_MPL_SEED_ID_LENGTH_2  = 1 << 6,
    IP6_OPTION_MPL_SEED_ID_LENGTH_8  = 2 << 6,
    IP6_OPTION_MPL_SEED_ID_LENGTH_16 = 3 << 6,
} ip6_option_mpl_seed_id_length_t;

enum {
    IP6_OPTION_MPL_SEED_ID_LENGTH_MASK = 3 << 6,
    IP6_OPTION_MPL_MAX_FLAG            = 1 << 5,
};

typedef struct _ip6_option_mpl {
    ip6_option_header_t option_header;
    uint8_t control;
    uint8_t sequence;
    uint16_t seed_id;
} ip6_option_mpl_t;

typedef struct _ip6_mpl_seed_entry {
    uint16_t seed_id;
    uint8_t sequence;
    uint8_t lifetime;
} ip6_mpl_seed_entry_t;

typedef struct _ip6_mpl_buffered_message_metadata {
    uint16_t seed_id;
    uint8_t sequence;
    uint8_t transmission_count;
    uint32_t transmission_time;
    uint8_t interval_offset;
} ip6_mpl_buffered_message_metadata_t;

enum {
    IP6_MPL_NUM_SEED_ENTRIES       = NS_CONFIG_MPL_SEED_SET_ENTRIES,
    IP6_MPL_SEED_ENTRY_LIFETIME    = NS_CONFIG_MPL_SEED_SET_ENTRY_LIFETIME,
    IP6_MPL_SEED_ENTRY_LIFETIME_DT = 1000,
    IP6_MPL_DATA_MESSAGE_INTERVAL  = 64,
};

typedef struct _ip6_mpl {
    void *instance;
    uint8_t timer_expirations;
    uint8_t sequence;
    uint16_t seed_id;
    timer_milli_t seed_set_timer;
    timer_milli_t retransmission_timer;
    const ip6_addr_t *matching_addr;
    ip6_mpl_seed_entry_t seed_set[IP6_MPL_NUM_SEED_ENTRIES];
    message_queue_t buffered_message_set;
} ip6_mpl_t;

// --- option mpl functions
void
ip6_option_mpl_init(ip6_option_mpl_t *ip6_option_mpl);

uint8_t
ip6_option_mpl_get_total_length(ip6_option_mpl_t *ip6_option_mpl);

ip6_option_mpl_seed_id_length_t
ip6_option_mpl_get_seed_id_length(ip6_option_mpl_t *ip6_option_mpl);

void
ip6_option_mpl_set_seed_id_length(ip6_option_mpl_t *ip6_option_mpl,
                                  ip6_option_mpl_seed_id_length_t seed_id_length);

bool
ip6_option_mpl_is_max_flag_set(ip6_option_mpl_t *ip6_option_mpl);

void
ip6_option_mpl_clear_max_flag(ip6_option_mpl_t *ip6_option_mpl);

void
ip6_option_mpl_set_max_flag(ip6_option_mpl_t *ip6_option_mpl);

uint8_t
ip6_option_mpl_get_sequence(ip6_option_mpl_t *ip6_option_mpl);

void
ip6_option_mpl_set_sequence(ip6_option_mpl_t *ip6_option_mpl, uint8_t sequence);

uint16_t
ip6_option_mpl_get_seed_id(ip6_option_mpl_t *ip6_option_mpl);

void
ip6_option_mpl_set_seed_id(ip6_option_mpl_t *ip6_option_mpl, uint16_t seed_id);

// --- mpl seed entry functions
uint16_t
ip6_mpl_seed_entry_get_seed_id(ip6_mpl_seed_entry_t *ip6_mpl_seed_entry);

void
ip6_mpl_seed_entry_set_seed_id(ip6_mpl_seed_entry_t *ip6_mpl_seed_entry, uint16_t seed_id);

uint8_t
ip6_mpl_seed_entry_get_sequence(ip6_mpl_seed_entry_t *ip6_mpl_seed_entry);

void
ip6_mpl_seed_entry_set_sequence(ip6_mpl_seed_entry_t *ip6_mpl_seed_entry, uint8_t sequence);

uint8_t
ip6_mpl_seed_entry_get_lifetime(ip6_mpl_seed_entry_t *ip6_mpl_seed_entry);

void
ip6_mpl_seed_entry_set_lifetime(ip6_mpl_seed_entry_t *ip6_mpl_seed_entry, uint8_t lifetime);

// --- mpl buffered message metadata functions
void
ip6_mpl_buffered_message_metadata_ctor(ip6_mpl_buffered_message_metadata_t *msg_metadata);

ns_error_t
ip6_mpl_buffered_message_metadata_append_to(ip6_mpl_buffered_message_metadata_t *msg_metadata,
                                            message_t message);

uint16_t
ip6_mpl_buffered_message_metadata_read_from(ip6_mpl_buffered_message_metadata_t *msg_metadata,
                                            const message_t message);

ns_error_t
ip6_mpl_buffered_message_metadata_remove_from(ip6_mpl_buffered_message_metadata_t *msg_metadata,
                                              message_t message); 

int
ip6_mpl_buffered_message_metadata_update_in(ip6_mpl_buffered_message_metadata_t *msg_metadata,
                                            message_t message);

bool
ip6_mpl_buffered_message_metadata_is_earlier(ip6_mpl_buffered_message_metadata_t *msg_metadata,
                                             uint32_t time);

bool
ip6_mpl_buffered_message_metadata_is_later(ip6_mpl_buffered_message_metadata_t *msg_metadata,
                                           uint32_t time);

uint16_t
ip6_mpl_buffered_message_metadata_get_seed_id(ip6_mpl_buffered_message_metadata_t *msg_metadata);

void
ip6_mpl_buffered_message_metadata_set_seed_id(ip6_mpl_buffered_message_metadata_t *msg_metadata,
                                              uint16_t seed_id);

uint8_t
ip6_mpl_buffered_message_metadata_get_sequence(ip6_mpl_buffered_message_metadata_t *msg_metadata);

void
ip6_mpl_buffered_message_metadata_set_sequence(ip6_mpl_buffered_message_metadata_t *msg_metadata,
                                               uint8_t sequence);

uint8_t
ip6_mpl_buffered_message_metadata_get_transmission_count(ip6_mpl_buffered_message_metadata_t *msg_metadata);

void
ip6_mpl_buffered_message_metadata_set_transmission_count(ip6_mpl_buffered_message_metadata_t *msg_metadata,
                                                         uint8_t transmission_count);

uint32_t
ip6_mpl_buffered_message_metadata_get_transmission_time(ip6_mpl_buffered_message_metadata_t *msg_metadata);

void
ip6_mpl_buffered_message_metadata_set_transmission_time(ip6_mpl_buffered_message_metadata_t *msg_metadata,
                                                        uint32_t transmission_time);

uint8_t
ip6_mpl_buffered_message_metadata_get_interval_offset(ip6_mpl_buffered_message_metadata_t *msg_metadata);

void
ip6_mpl_buffered_message_metadata_set_interval_offset(ip6_mpl_buffered_message_metadata_t *msg_metadata,
                                                      uint8_t interval_offset);

void
ip6_mpl_buffered_message_metadata_generate_next_transmission_time(ip6_mpl_buffered_message_metadata_t *
                                                                  msg_metadata,
                                                                  uint32_t current_time,
                                                                  uint8_t interval);

// --- ip6 mpl functions
void
ip6_mpl_ctor(ip6_mpl_t *ip6_mpl, void *instance);

void
ip6_mpl_init_option(ip6_mpl_t *ip6_mpl, ip6_option_mpl_t *option, const ip6_addr_t *addr);

ns_error_t
ip6_mpl_process_option(ip6_mpl_t *ip6_mpl, message_t message, const ip6_addr_t *addr, bool is_out_bound);

uint16_t
ip6_mpl_get_seed_id(ip6_mpl_t *ip6_mpl);

void
ip6_mpl_set_seed_id(ip6_mpl_t *ip6_mpl, uint16_t seed_id);

uint8_t
ip6_mpl_get_timer_expirations(ip6_mpl_t *ip6_mpl);

void
ip6_mpl_set_timer_expirations(ip6_mpl_t *ip6_mpl, uint8_t timer_expirations);

void
ip6_mpl_set_matching_addr(ip6_mpl_t *ip6_mpl, const ip6_addr_t *addr);

const message_queue_t *
ip6_mpl_get_buffered_message_set(ip6_mpl_t *ip6_mpl);

#endif // NS_CORE_NET_IP6_MPL_H_
