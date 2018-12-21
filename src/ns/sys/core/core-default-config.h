#ifndef NS_CORE_DEFAULT_CONFIG_H_
#define NS_CORE_DEFAULT_CONFIG_H_

// the organizationally uniqie identifier for the Nespy stack
#ifndef NS_CONFIG_STACK_VENDOR_OUI
#define NS_CONFIG_STACK_VENDOR_OUI 0x18b430
#endif

// the stack version revision for the Nespy stack
#ifndef NS_CONFIG_STACK_VERSION_REV
#define NS_CONFIG_STACK_VERSION_REV 0
#endif

// the stack version major for the Nespy stack
#ifndef NS_CONFIG_STACK_VERSION_MAJOR
#define NS_CONFIG_STACK_VERSION_MAJOR 0
#endif

// the stack version minor for the Nespy stack
#ifndef NS_CONFIG_STACK_VERSION_MINOR
#define NS_CONFIG_STACK_VERSION_MINOR 1
#endif

// the platform-specific string to insert into Nespy version string
#ifndef NS_CONFIG_PLATFORM_INFO
#define NS_CONFIG_PLATFORM_INFO "NONE"
#endif

// the number of message buffers in the buffer pool
#ifndef NS_CONFIG_MESSAGE_NUM_BUFFERS
#define NS_CONFIG_MESSAGE_NUM_BUFFERS 44
#endif

// the size of a message buffer in bytes
#ifndef NS_CONFIG_MESSAGE_BUFFER_SIZE
#define NS_CONFIG_MESSAGE_BUFFER_SIZE 128
#endif

// IEEE 802.15.4 default channel
#ifndef NS_CONFIG_DEFAULT_CHANNEL
#define NS_CONFIG_DEFAULT_CHANNEL 11
#endif

// define as 1 to enable time synchronization service feature
#ifndef NS_CONFIG_ENABLE_TIME_SYNC
#define NS_CONFIG_ENABLE_TIME_SYNC 0
#endif

// applicable only if time synchronization service feature is enable
// (i.e., NS_CONFIG_ENABLE_TIME_SYNC is set)
#ifndef NS_CONFIG_TIME_SYNC_REQUIRED
#define NS_CONFIG_TIME_SYNC_REQUIRED 0
#endif

// default period of time synchronization, in seconds
#ifndef NS_CONFIG_TIME_SYNC_PERIOD
#define NS_CONFIG_TIME_SYNC_PERIOD 30
#endif

// specifies the default xtal threshold for a device to become Router in time
// synchronization enabled network, in PPM.
#ifndef NS_CONFIG_TIME_SYNC_XTAL_THRESHOLD
#define NS_CONFIG_TIME_SYNC_XTAL_THRESHOLD 300
#endif

// define as 1 to support IEEE 802.15.4 Header IE (Information Element)
// generation and parsing
#ifndef NS_CONFIG_HEADER_IE_SUPPORT
#if NS_CONFIG_ENABLE_TIME_SYNC
#define NS_CONFIG_HEADER_IE_SUPPORT 1
#else
#define NS_CONFIG_HEADER_IE_SUPPORT 0
#endif
#endif

// the beacon version use when the beacon join flag is set.
#ifndef NS_CONFIG_JOIN_BEACON_VERSION
#define NS_CONFIG_JOIN_BEACON_VERSION MAC_BEACON_PAYLOAD_PROTOCOL_VERSION
#endif

// default heap size used for COAP and DTLS
#ifndef NS_CONFIG_HEAP_SIZE
#define NS_CONFIG_HEAP_SIZE (3072 * sizeof(void *))
#endif

// the maximum size of the CLI line in bytes
#ifndef NS_CONFIG_CLI_MAX_LINE_LEN
#define NS_CONFIG_CLI_MAX_LINE_LEN 128
#endif

// the size of CLI UART RX buffer in bytes
#ifndef NS_CONFIG_CLI_RX_BUFFER_SIZE
#define NS_CONFIG_CLI_RX_BUFFER_SIZE 512
#endif

// the size of CLI message buffer in bytes
#ifndef NS_CONFIG_CLI_TX_BUFFER_SIZE
#define NS_CONFIG_CLI_TX_BUFFER_SIZE 1024
#endif

// Note: `ns_plat_log()` is needed due to the default config for log output is
// platform defined
#ifndef NS_CONFIG_LOG_OUTPUT
#define NS_CONFIG_LOG_OUTPUT NS_CONFIG_LOG_OUTPUT_PLATFORM_DEFINED
#endif

// log output options
#define NS_CONFIG_LOG_OUTPUT_NONE 0
#define NS_CONFIG_LOG_OUTPUT_DEBUG_UART 1
#define NS_CONFIG_LOG_OUTPUT_APP 2
#define NS_CONFIG_LOG_OUTPUT_PLATFORM_DEFINED 3
#define NS_CONFIG_LOG_OUTPUT_NCP_SPINEL 4

#ifndef NS_CONFIG_LOG_LEVEL
#define NS_CONFIG_LOG_LEVEL NS_LOG_LEVEL_DEBG
#endif

#ifndef NS_CONFIG_ENABLE_DYNAMIC_LOG_LEVEL
#define NS_CONFIG_ENABLE_DYNAMIC_LOG_LEVEL 1
#endif

#ifndef NS_CONFIG_PLAT_LOG_FUNCTION
#define NS_CONFIG_PLAT_LOG_FUNCTION ns_plat_log
#endif

#ifndef NS_CONFIG_LOG_SUFFIX
#define NS_CONFIG_LOG_SUFFIX ""
#endif

#ifndef NS_CONFIG_ENABLE_DEBUG_UART
#define NS_CONFIG_ENABLE_DEBUG_UART 0
#endif

#ifndef NS_CONFIG_LOG_PKT_DUMP
#define NS_CONFIG_LOG_PKT_DUMP 1
#endif

#ifndef NS_CONFIG_LOG_PREPEND_LEVEL
#define NS_CONFIG_LOG_PREPEND_LEVEL 1
#endif

#ifndef NS_CONFIG_LOG_PREPEND_REGION
#define NS_CONFIG_LOG_PREPEND_REGION 1
#endif

// assert is managed by platform defined logic when this flag is set
#ifndef NS_CONFIG_PLATFORM_ASSERT_MANAGEMENT
#define NS_CONFIG_PLATFORM_ASSERT_MANAGEMENT 0
#endif

// only applicable only if raw link layer API is enabled
// (i.e., `NS_ENABLE_RAW_LINK_API` is set)
#ifndef NS_CONFIG_ENABLE_SOFTWARE_ACK_TIMEOUT
#define NS_CONFIG_ENABLE_SOFTWARE_ACK_TIMEOUT 1
#endif

#ifndef NS_CONFIG_ENABLE_SOFTWARE_ENERGY_SCAN
#define NS_CONFIG_ENABLE_SOFTWARE_ENERGY_SCAN 1
#endif

#ifndef NS_CONFIG_ENABLE_SOFTWARE_RETRANSMIT
#define NS_CONFIG_ENABLE_SOFTWARE_RETRANSMIT 1
#endif

#ifndef NS_CONFIG_ENABLE_SOFTWARE_CSMA_BACKOFF
#define NS_CONFIG_ENABLE_SOFTWARE_CSMA_BACKOFF 1
#endif

// define as 1 to disable CSMA-CA on the last transmit attempt
#ifndef NS_CONFIG_DISABLE_CSMA_CA_ON_LAST_ATTEMPT
#define NS_CONFIG_DISABLE_CSMA_CA_ON_LAST_ATTEMPT 0
#endif

// define to 1 if you want to enable microseconds backoff timer implemented in
// platform
#ifndef NS_CONFIG_ENABLE_PLATFORM_USEC_TIMER
#define NS_CONFIG_ENABLE_PLATFORM_USEC_TIMER 0
#endif

// maximum number of children
#ifndef NS_CONFIG_MAX_CHILDREN
#define NS_CONFIG_MAX_CHILDREN 10
#endif

// number of mac filter entries
#ifndef NS_CONFIG_MAC_FILTER_SIZE
#define NS_CONFIG_MAC_FILTER_SIZE 32
#endif

// the maximum number of backoffs the CSMA-CA algorithm will attempt before declaring a channel access failure.
// equivalent to macMaxCSMABackoffs in IEEE 802.15.4-2006, default value is 4
#ifndef NS_CONFIG_MAC_MAX_CSMA_BACKOFFS_DIRECT
#define NS_CONFIG_MAC_MAX_CSMA_BACKOFFS_DIRECT 32
#endif

// the maximum number of backoffs the CSMA-CA algorithm will attempt before declaring a channel access failure.
// equivalent to macMaxCSMABackoffs in IEEE 802.15.4-2006, default value is 4
#ifndef NS_CONFIG_MAC_MAX_CSMA_BACKOFFS_INDIRECT
#define NS_CONFIG_MAC_MAX_CSMA_BACKOFFS_INDIRECT 4
#endif

// the maximum number of retries allowed after transmission failure for direct transmission
// equivalent to macMaxFrameRetries, default value is 3
#ifndef NS_CONFIG_MAC_MAX_FRAME_RETRIES_DIRECT
#define NS_CONFIG_MAC_MAX_FRAME_RETRIES_DIRECT 3
#endif

// the maximum number of retries allowed after transmission failure for indirect transmission
// equivalent to macMaxFrameRetries, default value is 3
#ifndef NS_CONFIG_MAC_MAX_FRAME_RETRIES_INDIRECT
#define NS_CONFIG_MAC_MAX_FRAME_RETRIES_INDIRECT 0
#endif

// the number of times each IEEE 802.15.4 broadcast frame is transmitted.
// the minimum value is 1. Values larger than 1 may improve broadcast reliability by increasing redundancy,
// but may also increase congestion.
#ifndef NS_CONFIG_TX_NUM_BCAST
#define NS_CONFIG_TX_NUM_BCAST 1
#endif

// MAC implementation maintains the average failure rate of CCA (Clear Channel Assessment) operation on
// frame transmissions. This value specifies the window (in terms of number of transmissions or samples)
// over which the average rate is maintained.
// practically, the average value can be considered as the percentage of CCA failures in
// (approximately) last AVERAGING_WINDOW frame transmissions.
#ifndef NS_CONFIG_CCA_FAILURE_RATE_AVERAGING_WINDOW
#define NS_CONFIG_CCA_FAILURE_RATE_AVERAGING_WINDOW 512
#endif

// define as 1 to stay awake between fragments while transmitting a large packet,
// and to stay awake after receiving a packet with frame pending set to true.
#ifndef NS_CONFIG_STAY_AWAKE_BETWEEN_FRAGMENTS
#define NS_CONFIG_STAY_AWAKE_BETWEEN_FRAGMENTS 0
#endif

// define as 1 to enable transmission error rate tracking (for both MAC frames and IPv6 messages).
// when enabled, it will track average error rate of MAC frame transmissions and IPv6 message 
// error rate for every neighbor.
#ifndef NS_CONFIG_ENABLE_TX_ERROR_RATE_TRACKING
#define NS_CONFIG_ENABLE_TX_ERROR_RATE_TRACKING 1
#endif

#ifndef NS_CONFIG_FRAME_TX_ERR_RATE_AVERAGING_WINDOW
#define NS_CONFIG_FRAME_TX_ERR_RATE_AVERAGING_WINDOW 128
#endif

#ifndef NS_CONFIG_IPV6_TX_ERR_RATE_AVERAGING_WINDOW
#define NS_CONFIG_IPV6_TX_ERR_RATE_AVERAGING_WINDOW 128
#endif

#ifndef NS_CONFIG_MAX_STATECHANGE_HANDLERS
#define NS_CONFIG_MAX_STATECHANGE_HANDLERS 1
#endif

#endif // NS_CORE_DEFAULT_CONFIG_H_
