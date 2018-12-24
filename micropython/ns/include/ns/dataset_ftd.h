#ifndef NS_DATASET_FTD_H_
#define NS_DATASET_FTD_H_

#include "ns/dataset.h"
#include "ns/ip6.h"

ns_error_t
ns_dataset_set_active(ns_instance_t instance, const ns_operational_dataset_t *dataset);

ns_error_t
ns_dataset_set_pending(ns_instance_t instance, const ns_operational_dataset_t *dataset);

ns_error_t
ns_dataset_send_mgmt_active_get(ns_instance_t instance,
                                const ns_operational_dataset_components_t *dataset_components,
                                const uint8_t *tlv_types,
                                uint8_t length,
                                const ns_ip6_addr_t *addr);

ns_error_t
ns_dataset_send_mgmt_active_set(ns_instance_t instance,
                                const ns_operational_dataset_t *dataset,
                                const uint8_t *tlvs,
                                uint8_t length);

ns_error_t
ns_dataset_send_mgmt_pending_get(ns_instance_t instance,
                                 const ns_operational_dataset_components_t *dataset_components,
                                 const uint8_t *tlv_types,
                                 uint8_t length,
                                 const ns_ip6_addr_t *addr);

ns_error_t
ns_dataset_send_mgmt_pending_set(ns_instance_t instance,
                                 const ns_operational_dataset_t *dataset,
                                 const uint8_t *tlvs,
                                 uint8_t length);

uint16_t
ns_dataset_get_delay_timer_minimal(ns_instance_t instance);

ns_error_t
ns_dataset_set_delay_timer_minimal(ns_instance_t instance, uint32_t delay_timer_minimal);

#endif // NS_DATASET_FTD_H_
