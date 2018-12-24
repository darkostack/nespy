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

#endif // NS_COMMISSIONER_H_
