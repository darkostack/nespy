#ifndef NS_COMMISSIONER_H_
#define NS_COMMISSIONER_H_

#include "ns/dataset.h"
#include "ns/ip6.h"
#include "ns/platform/radio.h"

typedef enum _ns_commissioner_state
{
    NS_COMMISSIONER_STATE_DISABLED = 0, // Commissioner role is disabled.
    NS_COMMISSIONER_STATE_PETITION = 1, // Currently petitioning to become a Commissioner.
    NS_COMMISSIONER_STATE_ACTIVE   = 2, // Commissioner role is active.
} ns_commissioner_state_t;

#define NS_COMMISSIONING_PASSPHRASE_MIN_SIZE 6   // Minimum size of the Commissioning Passphrase
#define NS_COMMISSIONING_PASSPHRASE_MAX_SIZE 255 // Maximum size of the Commissioning Passphrase

#define NS_STEERING_DATA_MAX_LENGTH 16 // Max steering data length (bytes)

typedef struct _ns_steering_data
{
    uint8_t length;                          // Length of steering data (bytes)
    uint8_t m8[NS_STEERING_DATA_MAX_LENGTH]; // Byte values
} ns_steering_data_t;

typedef struct _ns_commissioning_dataset {
    uint16_t locator;
    uint16_t session_id;
    ns_steering_data_t steering_data;
    uint16_t joiner_udp_port;
    bool is_locator_set : 1;
    bool is_session_id_set : 1;
    bool is_steering_dataset : 1;
    bool is_joiner_udp_port_set : 1;
} ns_commissioning_dataset_t;

ns_error_t
ns_commissioner_start(ns_instance_t instance);

ns_error_t
ns_commissioner_stop(ns_instance_t instance);

ns_error_t
ns_commissioner_add_joiner(ns_instance_t instance,
                           const ns_ext_addr_t *eui64,
                           const char *pskd,
                           uint32_t timeout);

ns_error_t
ns_commissioner_remove_joiner(ns_instance_t instance, const ns_ext_addr_t *eui64);

const char *
ns_commissioner_get_provisioning_url(ns_instance_t instance, uint16_t *length);

ns_error_t
ns_commissioner_set_provisioning_url(ns_instance_t instance, const char *provisioning_url);

ns_error_t
ns_commissioner_announce_begin(ns_instance_t instance,
                               uint32_t channel_mask,
                               uint8_t count,
                               uint16_t period,
                               const ns_ip6_addr_t *addr);

typedef void (*ns_commissioner_energy_report_callback_func_t)(uint32_t channel_mask,
                                                              const uint8_t *energy_list,
                                                              uint8_t energy_list_length,
                                                              void *context);

ns_error_t
ns_commissioner_energy_scan(ns_instance_t instance,
                            uint32_t channel_mask,
                            uint8_t count,
                            uint16_t period,
                            uint16_t scan_duration,
                            const ns_ip6_addr_t *addr,
                            ns_commissioner_energy_report_callback_func_t callback,
                            void *context);

typedef void (*ns_commissioner_panid_conflict_callback_func_t)(uint16_t panid,
                                                               uint32_t channel_mask,
                                                               void *context);

ns_error_t
ns_commissioner_panid_query(ns_instance_t instance,
                            uint16_t panid,
                            uint32_t channel_mask,
                            const ns_ip6_addr_t *addr,
                            ns_commissioner_panid_conflict_callback_func_t callback,
                            void *context);

ns_error_t
ns_commissioner_send_mgmt_get(ns_instance_t instance, const uint8_t *tlvs, uint8_t length);

ns_error_t
ns_commissioner_send_mgmt_set(ns_instance_t instance,
                              const ns_commissioning_dataset_t *dataset,
                              const uint8_t *tlvs,
                              uint8_t length);

uint16_t
ns_commissioner_get_session_id(ns_instance_t instance);

ns_commissioner_state_t
ns_commissioner_get_state(ns_instance_t instance);

ns_error_t
ns_commissioner_generate_pskc(ns_instance_t instance,
                              const char *pass_phrase,
                              const char *network_name,
                              const ns_extended_panid_t *ext_panid,
                              uint8_t *pskc);

#endif // NS_COMMISSIONER_H_
