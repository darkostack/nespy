#include "core/net/ip6_mpl.h"
#include "core/common/code_utils.h"
#include "core/common/instance.h"
#include "core/common/random.h"
#include "core/net/ip6.h"

// --- private functions declarations
static ns_error_t
ip6_mpl_update_seed_set(ip6_mpl_t *ip6_mpl, uint16_t seed_id, uint8_t sequence);

static void
ip6_mpl_update_buffered_set(ip6_mpl_t *ip6_mpl, uint16_t seed_id, uint8_t sequence);

static void
ip6_mpl_add_buffered_message(ip6_mpl_t *ip6_mpl,
                             message_t message,
                             uint16_t seed_id,
                             uint8_t sequence,
                             bool is_out_bound);

static void
handle_seed_set_timer(void *timer);

static void
ip6_mpl_handle_seed_set_timer(ip6_mpl_t *ip6_mpl);

static void
handle_retransmission_timer(void *timer);

static void
ip6_mpl_handle_retransmission_timer(ip6_mpl_t *ip6_mpl);

// --- option mpl functions
void
ip6_option_mpl_init(ip6_option_mpl_t *ip6_option_mpl)
{
    ip6_option_header_set_type(&ip6_option_mpl->option_header, IP6_OPTION_MPL_TYPE);
    ip6_option_header_set_length(&ip6_option_mpl->option_header,
                                 sizeof(*ip6_option_mpl) - sizeof(ip6_option_header_t));
    ip6_option_mpl->control = 0;
}

uint8_t
ip6_option_mpl_get_total_length(ip6_option_mpl_t *ip6_option_mpl)
{
    return ip6_option_header_get_length(&ip6_option_mpl->option_header) + sizeof(ip6_option_header_t);
}

ip6_option_mpl_seed_id_length_t
ip6_option_mpl_get_seed_id_length(ip6_option_mpl_t *ip6_option_mpl)
{
    return (ip6_option_mpl_seed_id_length_t)(ip6_option_mpl->control & IP6_OPTION_MPL_SEED_ID_LENGTH_MASK);
}

void
ip6_option_mpl_set_seed_id_length(ip6_option_mpl_t *ip6_option_mpl,
                                  ip6_option_mpl_seed_id_length_t seed_id_length)
{
    ip6_option_mpl->control = (uint8_t)((ip6_option_mpl->control & ~IP6_OPTION_MPL_SEED_ID_LENGTH_MASK) | seed_id_length);
}

bool
ip6_option_mpl_is_max_flag_set(ip6_option_mpl_t *ip6_option_mpl)
{
    return (ip6_option_mpl->control & IP6_OPTION_MPL_MAX_FLAG) != 0;
}

void
ip6_option_mpl_clear_max_flag(ip6_option_mpl_t *ip6_option_mpl)
{
    ip6_option_mpl->control &= ~IP6_OPTION_MPL_MAX_FLAG;
}

void
ip6_option_mpl_set_max_flag(ip6_option_mpl_t *ip6_option_mpl)
{
    ip6_option_mpl->control |= IP6_OPTION_MPL_MAX_FLAG;
}

uint8_t
ip6_option_mpl_get_sequence(ip6_option_mpl_t *ip6_option_mpl)
{
    return ip6_option_mpl->sequence;
}

void
ip6_option_mpl_set_sequence(ip6_option_mpl_t *ip6_option_mpl, uint8_t sequence)
{
    ip6_option_mpl->sequence = sequence;
}

uint16_t
ip6_option_mpl_get_seed_id(ip6_option_mpl_t *ip6_option_mpl)
{
    return encoding_swap16(ip6_option_mpl->seed_id);
}

void
ip6_option_mpl_set_seed_id(ip6_option_mpl_t *ip6_option_mpl, uint16_t seed_id)
{
    ip6_option_mpl->seed_id = encoding_swap16(seed_id);
}

// --- mpl seed entry functions
uint16_t
ip6_mpl_seed_entry_get_seed_id(ip6_mpl_seed_entry_t *ip6_mpl_seed_entry)
{
    return ip6_mpl_seed_entry->seed_id;
}

void
ip6_mpl_seed_entry_set_seed_id(ip6_mpl_seed_entry_t *ip6_mpl_seed_entry, uint16_t seed_id)
{
    ip6_mpl_seed_entry->seed_id = seed_id;
}

uint8_t
ip6_mpl_seed_entry_get_sequence(ip6_mpl_seed_entry_t *ip6_mpl_seed_entry)
{
    return ip6_mpl_seed_entry->sequence;
}

void
ip6_mpl_seed_entry_set_sequence(ip6_mpl_seed_entry_t *ip6_mpl_seed_entry, uint8_t sequence)
{
    ip6_mpl_seed_entry->sequence = sequence;
}

uint8_t
ip6_mpl_seed_entry_get_lifetime(ip6_mpl_seed_entry_t *ip6_mpl_seed_entry)
{
    return ip6_mpl_seed_entry->lifetime;
}

void
ip6_mpl_seed_entry_set_lifetime(ip6_mpl_seed_entry_t *ip6_mpl_seed_entry, uint8_t lifetime)
{
    ip6_mpl_seed_entry->lifetime = lifetime;
}

// --- mpl buffered message metadata functions
void
ip6_mpl_buffered_message_metadata_ctor(ip6_mpl_buffered_message_metadata_t *msg_metadata)
{
    msg_metadata->seed_id = 0;
    msg_metadata->sequence = 0;
    msg_metadata->transmission_count = 0;
    msg_metadata->transmission_time = 0;
    msg_metadata->interval_offset = 0;
}

ns_error_t
ip6_mpl_buffered_message_metadata_append_to(ip6_mpl_buffered_message_metadata_t *msg_metadata,
                                            message_t message)
{
    return message_append(message, msg_metadata, sizeof(*msg_metadata));
}

uint16_t
ip6_mpl_buffered_message_metadata_read_from(ip6_mpl_buffered_message_metadata_t *msg_metadata,
                                            const message_t message)
{
    return message_read(message,
                        message_get_length(message) - sizeof(*msg_metadata),
                        msg_metadata,
                        sizeof(*msg_metadata));
}

ns_error_t
ip6_mpl_buffered_message_metadata_remove_from(ip6_mpl_buffered_message_metadata_t *msg_metadata,
                                              message_t message)
{
    return message_set_length(message, message_get_length(message) - sizeof(*msg_metadata));
}

int
ip6_mpl_buffered_message_metadata_update_in(ip6_mpl_buffered_message_metadata_t *msg_metadata,
                                            message_t message)
{
    return message_write(message,
                         message_get_length(message) - sizeof(*msg_metadata),
                         msg_metadata,
                         sizeof(*msg_metadata));
}

bool
ip6_mpl_buffered_message_metadata_is_earlier(ip6_mpl_buffered_message_metadata_t *msg_metadata,
                                             uint32_t time)
{
    return ((int32_t)(time - msg_metadata->transmission_time) > 0);
}

bool
ip6_mpl_buffered_message_metadata_is_later(ip6_mpl_buffered_message_metadata_t *msg_metadata,
                                           uint32_t time)
{
    return ((int32_t)(time - msg_metadata->transmission_time) < 0);
}

uint16_t
ip6_mpl_buffered_message_metadata_get_seed_id(ip6_mpl_buffered_message_metadata_t *msg_metadata)
{
    return msg_metadata->seed_id;
}

void
ip6_mpl_buffered_message_metadata_set_seed_id(ip6_mpl_buffered_message_metadata_t *msg_metadata,
                                              uint16_t seed_id)
{
    msg_metadata->seed_id = seed_id;
}

uint8_t
ip6_mpl_buffered_message_metadata_get_sequence(ip6_mpl_buffered_message_metadata_t *msg_metadata)
{
    return msg_metadata->sequence;
}

void
ip6_mpl_buffered_message_metadata_set_sequence(ip6_mpl_buffered_message_metadata_t *msg_metadata,
                                               uint8_t sequence)
{
    msg_metadata->sequence = sequence;
}

uint8_t
ip6_mpl_buffered_message_metadata_get_transmission_count(ip6_mpl_buffered_message_metadata_t *msg_metadata)
{
    return msg_metadata->transmission_count;
}

void
ip6_mpl_buffered_message_metadata_set_transmission_count(ip6_mpl_buffered_message_metadata_t *msg_metadata,
                                                         uint8_t transmission_count)
{
    msg_metadata->transmission_count = transmission_count;
}

uint32_t
ip6_mpl_buffered_message_metadata_get_transmission_time(ip6_mpl_buffered_message_metadata_t *msg_metadata)
{
    return msg_metadata->transmission_time;
}

void
ip6_mpl_buffered_message_metadata_set_transmission_time(ip6_mpl_buffered_message_metadata_t *msg_metadata,
                                                        uint32_t transmission_time)
{
    msg_metadata->transmission_time = transmission_time;
}

uint8_t
ip6_mpl_buffered_message_metadata_get_interval_offset(ip6_mpl_buffered_message_metadata_t *msg_metadata)
{
    return msg_metadata->interval_offset;
}

void
ip6_mpl_buffered_message_metadata_set_interval_offset(ip6_mpl_buffered_message_metadata_t *msg_metadata,
                                                      uint8_t interval_offset)
{
    msg_metadata->interval_offset = interval_offset;
}

void
ip6_mpl_buffered_message_metadata_generate_next_transmission_time(ip6_mpl_buffered_message_metadata_t *
                                                                  msg_metadata,
                                                                  uint32_t current_time,
                                                                  uint8_t interval)
{
    // emulate trickle timer behavior and set up the next retransmission wihin [0, I] range
    uint8_t t = interval == 0 ? interval : random_get_uint8_in_range(0, interval);

    // set transmission time at the beginning of the next interval
    ip6_mpl_buffered_message_metadata_set_transmission_time(msg_metadata,
            current_time + ip6_mpl_buffered_message_metadata_get_interval_offset(msg_metadata) + t);
    ip6_mpl_buffered_message_metadata_set_interval_offset(msg_metadata, interval - t);
}

// --- ip6 mpl functions
void
ip6_mpl_ctor(ip6_mpl_t *ip6_mpl, void *instance)
{
    ip6_mpl->instance = instance;
    ip6_mpl->timer_expirations = 0;
    ip6_mpl->sequence = 0;
    ip6_mpl->seed_id = 0;
    timer_ctor(instance, &ip6_mpl->seed_set_timer, &handle_seed_set_timer, ip6_mpl);
    timer_ctor(instance, &ip6_mpl->retransmission_timer, &handle_retransmission_timer, ip6_mpl);
    ip6_mpl->matching_addr = NULL;
    memset(ip6_mpl->seed_set, 0, sizeof(ip6_mpl->seed_set));
}

void
ip6_mpl_init_option(ip6_mpl_t *ip6_mpl, ip6_option_mpl_t *option, const ip6_addr_t *addr)
{
    ip6_option_mpl_init(option);
    ip6_option_mpl_set_sequence(option, ip6_mpl->sequence++);

    // check if seed id can be elided.
    if (ip6_mpl->matching_addr && ip6_addr_is_equal((ip6_addr_t *)ip6_mpl->matching_addr, addr)) {
        ip6_option_mpl_set_seed_id_length(option, IP6_OPTION_MPL_SEED_ID_LENGTH_0);
        // decrease default option length
        ip6_option_header_set_length(&option->option_header,
                ip6_option_header_get_length(&option->option_header) - sizeof(ip6_mpl->seed_id));
    } else {
        ip6_option_mpl_set_seed_id_length(option, IP6_OPTION_MPL_SEED_ID_LENGTH_2);
        ip6_option_mpl_set_seed_id(option, ip6_mpl->seed_id);
    }
}

ns_error_t
ip6_mpl_process_option(ip6_mpl_t *ip6_mpl, message_t message, const ip6_addr_t *addr, bool is_out_bound)
{
    ns_error_t error;
    ip6_option_mpl_t option;

    VERIFY_OR_EXIT(message_read(message, message_get_offset(message), &option, sizeof(option)) >=
                   IP6_OPTION_MPL_MIN_LENGTH &&
                   (ip6_option_mpl_get_seed_id_length(&option) == IP6_OPTION_MPL_SEED_ID_LENGTH_0 ||
                    ip6_option_mpl_get_seed_id_length(&option) == IP6_OPTION_MPL_SEED_ID_LENGTH_2),
                   error = NS_ERROR_DROP);

    if (ip6_option_mpl_get_seed_id_length(&option) == IP6_OPTION_MPL_SEED_ID_LENGTH_0) {
        // retrieve MPL seed id from the IPv6 source address
        ip6_option_mpl_set_seed_id(&option, encoding_swap16(addr->fields.m16[7]));
    }

    // check MPL data messages in the MPL buffered set against sequence number
    ip6_mpl_update_buffered_set(ip6_mpl,
                                ip6_option_mpl_get_seed_id(&option),
                                ip6_option_mpl_get_sequence(&option));

    // check if the MPL data message is new
    error = ip6_mpl_update_seed_set(ip6_mpl,
                                    ip6_option_mpl_get_seed_id(&option),
                                    ip6_option_mpl_get_sequence(&option));

    if (error == NS_ERROR_NONE) {
        ip6_mpl_add_buffered_message(ip6_mpl,
                                     message, 
                                     ip6_option_mpl_get_seed_id(&option),
                                     ip6_option_mpl_get_sequence(&option),
                                     is_out_bound);
    } else if (is_out_bound) {
        // In case MPL Data Message is generated locally, ignore potential error of the MPL Seed Set
        // to allow subsequent retransmissions with the same sequence number.
        EXIT_NOW(error = NS_ERROR_NONE);
    }

exit:
    return error;
}

uint16_t
ip6_mpl_get_seed_id(ip6_mpl_t *ip6_mpl)
{
    return encoding_swap16(ip6_mpl->seed_id);
}

void
ip6_mpl_set_seed_id(ip6_mpl_t *ip6_mpl, uint16_t seed_id)
{
    ip6_mpl->seed_id = encoding_swap16(seed_id);
}

uint8_t
ip6_mpl_get_timer_expirations(ip6_mpl_t *ip6_mpl)
{
    return ip6_mpl->timer_expirations;
}

void
ip6_mpl_set_timer_expirations(ip6_mpl_t *ip6_mpl, uint8_t timer_expirations)
{
    ip6_mpl->timer_expirations = timer_expirations;
}

void
ip6_mpl_set_matching_addr(ip6_mpl_t *ip6_mpl, const ip6_addr_t *addr)
{
    ip6_mpl->matching_addr = addr;
}

const message_queue_t *
ip6_mpl_get_buffered_message_set(ip6_mpl_t *ip6_mpl)
{
    return &ip6_mpl->buffered_message_set;
}

// --- private functions
static ns_error_t
ip6_mpl_update_seed_set(ip6_mpl_t *ip6_mpl, uint16_t seed_id, uint8_t sequence)
{
    ns_error_t error = NS_ERROR_NONE;
    ip6_mpl_seed_entry_t *entry = NULL;
    int8_t diff;
    uint32_t i = 0;

    for (i = 0; i < IP6_MPL_NUM_SEED_ENTRIES; i++) {
        if (ip6_mpl_seed_entry_get_lifetime(&ip6_mpl->seed_set[i]) == 0) {
            // start allocating from the first possible entry to the speed up process of searching
            if (entry == NULL) {
                entry = &ip6_mpl->seed_set[i];
            }
        } else if (ip6_mpl_seed_entry_get_seed_id(&ip6_mpl->seed_set[i]) == seed_id) {
            entry = &ip6_mpl->seed_set[i];
            diff = (int8_t)(sequence - ip6_mpl_seed_entry_get_sequence(entry));
            VERIFY_OR_EXIT(diff > 0, error = NS_ERROR_DROP);
            break;
        }
    }

    VERIFY_OR_EXIT(entry != NULL, error = NS_ERROR_DROP);

    ip6_mpl_seed_entry_set_seed_id(&ip6_mpl->seed_set[i], seed_id);
    ip6_mpl_seed_entry_set_sequence(&ip6_mpl->seed_set[i], sequence);
    ip6_mpl_seed_entry_set_lifetime(&ip6_mpl->seed_set[i], IP6_MPL_SEED_ENTRY_LIFETIME);

    timer_milli_start(&ip6_mpl->seed_set_timer, IP6_MPL_SEED_ENTRY_LIFETIME_DT);

exit:
    return error;
}

static void
ip6_mpl_update_buffered_set(ip6_mpl_t *ip6_mpl, uint16_t seed_id, uint8_t sequence)
{
    int8_t diff;
    ip6_mpl_buffered_message_metadata_t message_metadata;
    ip6_mpl_buffered_message_metadata_ctor(&message_metadata);

    message_t message = message_queue_get_head(&ip6_mpl->buffered_message_set);
    message_t next_message = NULL;

    // check if multicast forwarding is enabled
    VERIFY_OR_EXIT(ip6_mpl_get_timer_expirations(ip6_mpl) > 0);

    while (message != NULL) {
        next_message = message_get_next(message);
        ip6_mpl_buffered_message_metadata_read_from(&message_metadata, (const message_t)message);
        if (ip6_mpl_buffered_message_metadata_get_seed_id(&message_metadata) == seed_id) {
            diff = (int8_t)(sequence - ip6_mpl_buffered_message_metadata_get_sequence(&message_metadata));
            if (diff > 0) {
                // stop retransmitting MPL data message that is consider to be old
                message_queue_dequeue(&ip6_mpl->buffered_message_set, message);
                message_free(message);
            }
            break;
        }
        message = next_message;
    }

exit:
    return;
}

static void
ip6_mpl_add_buffered_message(ip6_mpl_t *ip6_mpl,
                             message_t message,
                             uint16_t seed_id,
                             uint8_t sequence,
                             bool is_out_bound)
{
    uint32_t now = timer_milli_get_now();
    ns_error_t error = NS_ERROR_NONE;
    message_t message_copy = NULL;
    ip6_mpl_buffered_message_metadata_t message_metadata;
    ip6_mpl_buffered_message_metadata_ctor(&message_metadata);
    uint32_t next_transmission_time;
    uint8_t hop_limit = 0;

#if NS_CONFIG_ENABLE_DYNAMIC_MPL_INTERVAL
    // TODO: adjust the first MPL forward interval dynamically according to the network scale
#else
    uint8_t interval = IP6_MPL_DATA_MESSAGE_INTERVAL;
#endif

    VERIFY_OR_EXIT(ip6_mpl_get_timer_expirations(ip6_mpl) > 0);
    VERIFY_OR_EXIT((message_copy = message_clone(message)) != NULL, error = NS_ERROR_NO_BUFS);

    if (!is_out_bound) {
        message_read(message, ip6_header_get_hop_limit_offset(), &hop_limit, ip6_header_get_hop_limit_size());
        VERIFY_OR_EXIT(hop_limit-- > 1, error = NS_ERROR_DROP);
        message_write(message_copy,
                      ip6_header_get_hop_limit_offset(),
                      &hop_limit,
                      ip6_header_get_hop_limit_size());
    }

    ip6_mpl_buffered_message_metadata_set_seed_id(&message_metadata, seed_id);
    ip6_mpl_buffered_message_metadata_set_sequence(&message_metadata, sequence);
    ip6_mpl_buffered_message_metadata_set_transmission_count(&message_metadata, is_out_bound ? 1 : 0);
    ip6_mpl_buffered_message_metadata_generate_next_transmission_time(&message_metadata, now, interval);

    // append the message with ip6_mpl_buffered_message_metadata_t and add it to the queue
    SUCCESS_OR_EXIT(error = ip6_mpl_buffered_message_metadata_append_to(&message_metadata, message_copy));
    message_queue_enqueue(&ip6_mpl->buffered_message_set, message_copy, MESSAGE_QUEUE_POS_TAIL);

    if (timer_is_running(&ip6_mpl->retransmission_timer)) {
        // if timer is already running, check if it should be restarted with earlier firetime
        next_transmission_time = timer_get_firetime(&ip6_mpl->retransmission_timer);
        if (ip6_mpl_buffered_message_metadata_is_earlier(&message_metadata, next_transmission_time)) {
            timer_milli_start(&ip6_mpl->retransmission_timer,
                              ip6_mpl_buffered_message_metadata_get_transmission_time(&message_metadata) - now);
        }
    } else {
        // otherwise just set the timer
        timer_milli_start(&ip6_mpl->retransmission_timer,
                          ip6_mpl_buffered_message_metadata_get_transmission_time(&message_metadata) - now);
    }

exit:
    if (error != NS_ERROR_NONE && message_copy != NULL) {
        message_free(message_copy);
    }
}

static void
handle_seed_set_timer(void *timer)
{
    ip6_mpl_t *ip6_mpl = (ip6_mpl_t *)((timer_t *)timer)->handler.context;
    ip6_mpl_handle_seed_set_timer(ip6_mpl);
}

static void
ip6_mpl_handle_seed_set_timer(ip6_mpl_t *ip6_mpl)
{
    bool start_timer = false;

    for (int i = 0; i < IP6_MPL_NUM_SEED_ENTRIES; i++) {
        if (ip6_mpl_seed_entry_get_lifetime(&ip6_mpl->seed_set[i]) > 0) {
            ip6_mpl_seed_entry_set_lifetime(&ip6_mpl->seed_set[i],
                                            ip6_mpl_seed_entry_get_lifetime(&ip6_mpl->seed_set[i]) - 1);
            start_timer = true;
        }
    }

    if (start_timer) {
        timer_milli_start(&ip6_mpl->seed_set_timer, IP6_MPL_SEED_ENTRY_LIFETIME_DT);
    }
}

static void
handle_retransmission_timer(void *timer)
{
    ip6_mpl_t *ip6_mpl = (ip6_mpl_t *)((timer_t *)timer)->handler.context;
    ip6_mpl_handle_retransmission_timer(ip6_mpl);
}

static void
ip6_mpl_handle_retransmission_timer(ip6_mpl_t *ip6_mpl)
{
    uint32_t now = timer_milli_get_now();
    uint32_t next_delta = 0xffffffff;
    ip6_mpl_buffered_message_metadata_t message_metadata;
    ip6_mpl_buffered_message_metadata_ctor(&message_metadata);

    message_t message = message_queue_get_head(&ip6_mpl->buffered_message_set);
    message_t next_message = NULL;

    while (message != NULL) {
        next_message = message_get_next(message);
        ip6_mpl_buffered_message_metadata_read_from(&message_metadata, message);
        if (ip6_mpl_buffered_message_metadata_is_later(&message_metadata, now)) {
            // calculate the next retransmission time and choose the lowest
            if (ip6_mpl_buffered_message_metadata_get_transmission_time(&message_metadata) - now < next_delta) {
                next_delta = ip6_mpl_buffered_message_metadata_get_transmission_time(&message_metadata) - now;
            }
        } else {
            // update the number of transmission timer expirations
            ip6_mpl_buffered_message_metadata_set_transmission_count(&message_metadata,
                    ip6_mpl_buffered_message_metadata_get_transmission_count(&message_metadata) + 1);
            if (ip6_mpl_buffered_message_metadata_get_transmission_count(&message_metadata) <
                ip6_mpl_get_timer_expirations(ip6_mpl)) {
                message_t message_copy = message_clone_length(message,
                        message_get_length(message) - sizeof(ip6_mpl_buffered_message_metadata_t));
                if (message_copy != NULL) {
                    if (ip6_mpl_buffered_message_metadata_get_transmission_count(&message_metadata) > 1) {
                        message_set_sub_type(message_copy, MESSAGE_SUB_TYPE_MPL_RETRANSMISSION);
                    }
                    ip6_enqueue_datagram(instance_get_ip6(ip6_mpl->instance), message_copy);
                }
                ip6_mpl_buffered_message_metadata_generate_next_transmission_time(&message_metadata,
                        now, IP6_MPL_DATA_MESSAGE_INTERVAL);
                ip6_mpl_buffered_message_metadata_update_in(&message_metadata, message);
                // check if retransmission time is lower than the current lowest one
                if (ip6_mpl_buffered_message_metadata_get_transmission_time(&message_metadata) - now <
                    next_delta) {
                    next_delta = ip6_mpl_buffered_message_metadata_get_transmission_time(&message_metadata) - now;
                }
            } else {
                message_queue_dequeue(&ip6_mpl->buffered_message_set, message);
                if (ip6_mpl_buffered_message_metadata_get_transmission_time(&message_metadata) ==
                    ip6_mpl_get_timer_expirations(ip6_mpl)) {
                    if (ip6_mpl_buffered_message_metadata_get_transmission_count(&message_metadata) > 1) {
                        message_set_sub_type(message, MESSAGE_SUB_TYPE_MPL_RETRANSMISSION);
                    }

                    // remove the extra metadata from MPL data message
                    ip6_mpl_buffered_message_metadata_read_from(&message_metadata, message);
                    ip6_enqueue_datagram(instance_get_ip6(ip6_mpl->instance), message);

                } else {
                    // stop retransmitting if the number of timer expirations is already exceeded
                    message_free(message);
                }
            }
        }
        message = next_message;
    }

    if (next_delta != 0xffffffff) {
        timer_milli_start(&ip6_mpl->retransmission_timer, next_delta);
    }
}
