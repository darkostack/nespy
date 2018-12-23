#ifndef NS_LINK_RAW_H_
#define NS_LINK_RAW_H_

#include "ns/platform/radio.h"

ns_error_t
ns_link_raw_set_enable(ns_instance_t instance, bool enabled);

bool
ns_link_raw_is_enabled(ns_instance_t instance);

bool
ns_link_raw_get_promiscuous(ns_instance_t instance);

ns_error_t
ns_link_raw_set_promiscuous(ns_instance_t instance);

ns_error_t
ns_link_raw_sleep(ns_instance_t instance);

typedef void (*ns_link_raw_receive_done_func_t)(ns_instance_t instance,
                                                ns_radio_frame_t *frame,
                                                ns_error_t error);

ns_error_t
ns_link_raw_receive(ns_instance_t instance, ns_link_raw_receive_done_func_t callback);

ns_radio_frame_t *
ns_link_raw_get_transmit_buffer(ns_instance_t instance);


typedef void (*ns_link_raw_transmit_done_func_t)(ns_instance_t instance,
                                                 ns_radio_frame_t *frame,
                                                 ns_radio_frame_t *ack_frame,
                                                 ns_error_t error);

ns_error_t
ns_link_raw_transmit(ns_instance_t instance, ns_link_raw_transmit_done_func_t callback);

int8_t
ns_link_raw_get_rssi(ns_instance_t instance);

ns_radio_caps_t
ns_link_raw_get_caps(ns_instance_t instance);

typedef void (*ns_link_raw_energy_scan_done_func_t)(ns_instance_t instance,
                                                    int8_t energy_scan_max_rssi);

ns_error_t
ns_link_raw_energy_scan(ns_instance_t instance,
                        uint8_t scan_channel,
                        uint16_t scan_duration,
                        ns_link_raw_energy_scan_done_func_t callback);

ns_error_t
ns_link_raw_src_match_enable(ns_instance_t instance, bool enable);

ns_error_t
ns_link_raw_src_match_add_short_entry(ns_instance_t instance, const uint16_t short_address);

ns_error_t
ns_link_raw_src_match_add_ext_entry(ns_instance_t instance, const ns_ext_addr_t *ext_address);

ns_error_t
ns_link_raw_src_match_clear_short_entry(ns_instance_t instance, const uint16_t short_address);

ns_error_t
ns_link_raw_src_match_clear_ext_entry(ns_instance_t instance, const ns_ext_addr_t *ext_address);

ns_error_t
ns_link_raw_src_match_clear_short_entries(ns_instance_t instance);

ns_error_t
ns_link_raw_src_match_clear_ext_entries(ns_instance_t instance);

#endif // NS_LINK_RAW_H_
