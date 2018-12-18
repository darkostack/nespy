#include "ns/include/dataset.h"
#include "ns/include/platform/alarm-milli.h"
#include "ns/include/platform/alarm-micro.h"
#include "ns/include/platform/radio.h"
#include "ns/include/platform/random.h"
#include "ns/include/platform/time.h"
#include "ns/sys/core/common/code_utils.h"
#include "platform-unix.h"

enum {
    IEEE802154_MIN_LENGTH = 5,
    IEEE802154_MAX_LENGTH = 127,
    IEEE802154_ACK_LENGTH = 5,

    IEEE802154_BROADCAST = 0xffff,

    IEEE802154_FRAME_TYPE_ACK    = 2 << 0,
    IEEE802154_FRAME_TYPE_MACCMD = 3 << 0,
    IEEE802154_FRAME_TYPE_MASK   = 7 << 0,

    IEEE802154_SECURITY_ENABLED  = 1 << 3,
    IEEE802154_FRAME_PENDING     = 1 << 4,
    IEEE802154_ACK_REQUEST       = 1 << 5,
    IEEE802154_PANID_COMPRESSION = 1 << 6,

    IEEE802154_DST_ADDR_NONE  = 0 << 2,
    IEEE802154_DST_ADDR_SHORT = 2 << 2,
    IEEE802154_DST_ADDR_EXT   = 3 << 2,
    IEEE802154_DST_ADDR_MASK  = 3 << 2,

    IEEE802154_SRC_ADDR_NONE  = 0 << 6,
    IEEE802154_SRC_ADDR_SHORT = 2 << 6,
    IEEE802154_SRC_ADDR_EXT   = 3 << 6,
    IEEE802154_SRC_ADDR_MASK  = 3 << 6,

    IEEE802154_DSN_OFFSET     = 2,
    IEEE802154_DSTPAN_OFFSET  = 3,
    IEEE802154_DSTADDR_OFFSET = 5,

    IEEE802154_SEC_LEVEL_MASK = 7 << 0,

    IEEE802154_KEY_ID_MODE_0    = 0 << 3,
    IEEE802154_KEY_ID_MODE_1    = 1 << 3,
    IEEE802154_KEY_ID_MODE_2    = 2 << 3,
    IEEE802154_KEY_ID_MODE_3    = 3 << 3,
    IEEE802154_KEY_ID_MODE_MASK = 3 << 3,

    IEEE802154_MACCMD_DATA_REQ = 4,
};

enum {
    UNIX_RECEIVE_SENSITIVITY   = -100, // dBm
    UNIX_MAX_SRC_MATCH_ENTRIES = NS_CONFIG_MAX_CHILDREN,

    UNIX_HIGH_RSSI_SAMPLE               = -30, // dBm
    UNIX_LOW_RSSI_SAMPLE                = -98, // dBm
    UNIX_HIGH_RSSI_PROB_INC_PER_CHANNEL = 5,
};

struct _radio_message {
    uint8_t channel;
    uint8_t psdu[NS_RADIO_FRAME_MAX_SIZE];
};

static ns_radio_state_t      s_state = NS_RADIO_STATE_DISABLED;
static struct _radio_message s_receive_message;
static struct _radio_message s_transmit_message;
static struct _radio_message s_ack_message;
static ns_radio_frame_t      s_receive_frame;
static ns_radio_frame_t      s_transmit_frame;
static ns_radio_frame_t      s_ack_frame;

#if NS_CONFIG_HEADER_IE_SUPPORT
static ns_radio_ie_info_t s_transmit_ie_info;
static ns_radio_ie_info_t s_received_ie_info;
#endif

static uint8_t  s_extended_addr[NS_EXT_ADDRESS_SIZE];
static uint16_t s_short_addr;
static uint16_t s_panid;
static uint16_t s_port_offset = 0;
static int      s_sock_fd;
static bool     s_promiscuous = false;
static bool     s_ack_wait = false;
static int8_t   s_tx_power = 0;

static uint8_t       s_short_addr_match_table_count = 0;
static uint8_t       s_ext_addr_match_table_count = 0;
static uint16_t      s_short_addr_match_table[UNIX_MAX_SRC_MATCH_ENTRIES];
static ns_ext_addr_t s_ext_addr_match_table[UNIX_MAX_SRC_MATCH_ENTRIES];
static bool          s_src_match_enabled = false;

// --- private functions declarations
static bool
find_short_address(uint16_t short_addr);

static bool
find_ext_address(const ns_ext_addr_t *ext_addr);

static bool
is_frame_type_ack(const uint8_t *frame);

static bool
is_frame_type_mac_cmd(const uint8_t *frame);

static bool
is_security_enabled(const uint8_t *frame);

static bool
is_ack_requested(const uint8_t *frame);

static bool
is_panid_compressed(const uint8_t *frame);

static bool
is_data_request_and_has_frame_pending(const uint8_t *frame);

static uint8_t
get_dsn(const uint8_t *frame);

static ns_panid_t
get_dsn_pan(const uint8_t *frame);

static ns_short_addr_t
get_short_addr(const uint8_t *frame);

static void
get_ext_addr(const uint8_t *frame, ns_ext_addr_t *addr);

static uint16_t
crc16_citt(uint16_t fcs, uint8_t byte);

static void
radio_transmit(struct _radio_message *msg, const struct _ns_radio_frame *pkt);

static void
radio_send_message(ns_instance_t instance);

static void
radio_send_ack(void);

static void
radio_process_frame(ns_instance_t instance);

// --- radio functions
void
ns_plat_radio_get_ieee_eui64(ns_instance_t instance, uint8_t *ieee_eui64)
{
    (void)instance;
    ieee_eui64[0] = 0x18;
    ieee_eui64[1] = 0xb4;
    ieee_eui64[2] = 0x30;
    ieee_eui64[3] = 0x00;
    ieee_eui64[4] = (g_node_id >> 24) & 0xff;
    ieee_eui64[5] = (g_node_id >> 16) & 0xff;
    ieee_eui64[6] = (g_node_id >> 8) & 0xff;
    ieee_eui64[7] = g_node_id & 0xff;
}

void
ns_plat_radio_set_panid(ns_instance_t instance, uint16_t panid)
{
    (void)instance;
    s_panid = panid;
}

// --- private functions
static bool
find_short_address(uint16_t short_addr)
{
    uint8_t i;
    for (i = 0; i < short_addr_match_table_count; ++i) {
        if (short_addr_match_table[i] == short_addr) {
            break;
        }
    }
    return i < short_addr_match_table_count;
}

static bool
find_ext_address(const ns_ext_addr_t *ext_addr)
{
    uint8_t i;
    for (i = 0; i < ext_addr_match_table_count; ++i) {
        if (!memcmp(&ext_addr_match_table[i], ext_addr, sizeof(ns_ext_addr_t))) {
            break;
        }
    }
    return i < ext_addr_match_table_count;
}

static bool
is_frame_type_ack(const uint8_t *frame)
{
    return (frame[0] & IEEE802154_FRAME_TYPE_MASK) == IEEE802154_FRAME_TYPE_ACK;
}

static bool
is_frame_type_mac_cmd(const uint8_t *frame)
{
    return (frame[0] & IEEE802154_FRAME_TYPE_MASK) == IEEE802154_FRAME_TYPE_MACCMD;
}

static bool
is_security_enabled(const uint8_t *frame)
{
    return (frame[0] & IEEE802154_SECURITY_ENABLED) != 0;
}

static bool
is_ack_requested(const uint8_t *frame)
{
    return (frame[0] & IEEE802154_ACK_REQUEST) != 0;
}

static bool
is_panid_compressed(const uint8_t *frame)
{
    return (frame[0] & IEEE802154_PANID_COMPRESSION) != 0;
}

static bool
is_data_request_and_has_frame_pending(const uint8_t *frame)
{
    const uint8_t *cur = frame;
    uint8_t security_control;
    bool is_data_request = false;
    bool has_frame_pending = false;

    // FCF + DSN
    cur += 2 + 1;

    EXPECT(is_frame_type_mac_cmd(frame));

    // destination PAN + address
    switch (frame[1] & IEEE802154_DST_ADDR_MASK) {
    case IEEE802154_DST_ADDR_SHORT:
        cur += sizeof(ns_panid_t) + sizeof(ns_short_addr_t);
        break;
    case IEEE802154_DST_ADDR_EXT:
        cur += sizeof(ns_panid_t) + sizeof(ns_ext_addr_t);
        break;
    default:
        goto exit;
    }

    // source PAN + address
    switch (frame[1] & IEEE802154_SRC_ADDR_MASK) {
    case IEEE802154_SRC_ADDR_SHORT:
        if (!is_panid_compressed(frame)) {
            cur += sizeof(ns_panid_t);
        }
        if (src_match_enabled) {
            has_frame_pending = find_short_address((uint16_t)(cur[1] << 8 | cur[0]));
        }
        cur += sizeof(ns_short_addr_t);
        break;
    case IEEE802154_SRC_ADDR_EXT:
        if (!is_panid_compressed(frame)) {
            cur += sizeof(ns_panid_t);
        }
        if (src_match_enabled) {
            has_frame_pending = find_ext_address((const ns_ext_addr_t *)cur);
        }
        cur += sizeof(ns_ext_addr_t);
        break;
    default:
        goto exit;
    }

    // security control + frame counter + key identifier
    if (is_security_enabled(frame)) {
        security_control = *cur;
        if (security_control & IEEE802154_SEC_LEVEL_MASK) {
            cur += 1 + 4;
        }
        switch (security_control & IEEE802154_KEY_ID_MODE_MASK) {
        case IEEE802154_KEY_ID_MODE_0:
            cur += 0;
            break;
        case IEEE802154_KEY_ID_MODE_1:
            cur += 1;
            break;
        case IEEE802154_KEY_ID_MODE_2:
            cur += 5;
            break;
        case IEEE802154_KEY_ID_MODE_3:
            cur += 9;
            break;
        }
    }

    // command ID
    is_data_request = cur[0] == IEEE802154_MACCMD_DATA_REQ;

exit:
    return is_data_request && has_frame_pending;
}

static uint8_t
get_dsn(const uint8_t *frame)
{
    return frame[IEEE802154_DSN_OFFSET];
}

static ns_panid_t
get_dsn_pan(const uint8_t *frame)
{
    return (ns_panid_t)((frame[IEEE802154_DSTPAN_OFFSET + 1] << 8) | frame[IEEE802154_DSTPAN_OFFSET]);
}

static ns_short_addr_t
get_short_addr(const uint8_t *frame)
{
    return (ns_short_addr_t)((frame[IEEE802154_DSTADDR_OFFSET + 1] << 8) | frame[IEEE802154_DSTADDR_OFFSET]);
}

static void
get_ext_addr(const uint8_t *frame, ns_ext_addr_t *addr)
{
    size_t i;
    for (i = 0; i < sizeof(ns_ext_addr_t); i++) {
        addr->m8[i] = frame[IEEE802154_DSTADDR_OFFSET + (sizeof(ns_ext_addr_t) - 1 - i)];
    }
}

static uint16_t
crc16_citt(uint16_t fcs, uint8_t byte)
{
    // CRC-16/CCITT, CRC-16/CCITT-TRUE, CRC-CCITT
    // width=16 poly=0x1021 init=0x0000 refin=true refout=true xorout=0x0000 check=0x2189 name="KERMIT"
    // http://reveng.sourceforge.net/crc-catalogue/16.htm#crc.cat.kermit
    static const uint16_t fcs_table[256] = {
        0x0000, 0x1189, 0x2312, 0x329b, 0x4624, 0x57ad, 0x6536, 0x74bf, 0x8c48, 0x9dc1, 0xaf5a, 0xbed3, 0xca6c, 0xdbe5,
        0xe97e, 0xf8f7, 0x1081, 0x0108, 0x3393, 0x221a, 0x56a5, 0x472c, 0x75b7, 0x643e, 0x9cc9, 0x8d40, 0xbfdb, 0xae52,
        0xdaed, 0xcb64, 0xf9ff, 0xe876, 0x2102, 0x308b, 0x0210, 0x1399, 0x6726, 0x76af, 0x4434, 0x55bd, 0xad4a, 0xbcc3,
        0x8e58, 0x9fd1, 0xeb6e, 0xfae7, 0xc87c, 0xd9f5, 0x3183, 0x200a, 0x1291, 0x0318, 0x77a7, 0x662e, 0x54b5, 0x453c,
        0xbdcb, 0xac42, 0x9ed9, 0x8f50, 0xfbef, 0xea66, 0xd8fd, 0xc974, 0x4204, 0x538d, 0x6116, 0x709f, 0x0420, 0x15a9,
        0x2732, 0x36bb, 0xce4c, 0xdfc5, 0xed5e, 0xfcd7, 0x8868, 0x99e1, 0xab7a, 0xbaf3, 0x5285, 0x430c, 0x7197, 0x601e,
        0x14a1, 0x0528, 0x37b3, 0x263a, 0xdecd, 0xcf44, 0xfddf, 0xec56, 0x98e9, 0x8960, 0xbbfb, 0xaa72, 0x6306, 0x728f,
        0x4014, 0x519d, 0x2522, 0x34ab, 0x0630, 0x17b9, 0xef4e, 0xfec7, 0xcc5c, 0xddd5, 0xa96a, 0xb8e3, 0x8a78, 0x9bf1,
        0x7387, 0x620e, 0x5095, 0x411c, 0x35a3, 0x242a, 0x16b1, 0x0738, 0xffcf, 0xee46, 0xdcdd, 0xcd54, 0xb9eb, 0xa862,
        0x9af9, 0x8b70, 0x8408, 0x9581, 0xa71a, 0xb693, 0xc22c, 0xd3a5, 0xe13e, 0xf0b7, 0x0840, 0x19c9, 0x2b52, 0x3adb,
        0x4e64, 0x5fed, 0x6d76, 0x7cff, 0x9489, 0x8500, 0xb79b, 0xa612, 0xd2ad, 0xc324, 0xf1bf, 0xe036, 0x18c1, 0x0948,
        0x3bd3, 0x2a5a, 0x5ee5, 0x4f6c, 0x7df7, 0x6c7e, 0xa50a, 0xb483, 0x8618, 0x9791, 0xe32e, 0xf2a7, 0xc03c, 0xd1b5,
        0x2942, 0x38cb, 0x0a50, 0x1bd9, 0x6f66, 0x7eef, 0x4c74, 0x5dfd, 0xb58b, 0xa402, 0x9699, 0x8710, 0xf3af, 0xe226,
        0xd0bd, 0xc134, 0x39c3, 0x284a, 0x1ad1, 0x0b58, 0x7fe7, 0x6e6e, 0x5cf5, 0x4d7c, 0xc60c, 0xd785, 0xe51e, 0xf497,
        0x8028, 0x91a1, 0xa33a, 0xb2b3, 0x4a44, 0x5bcd, 0x6956, 0x78df, 0x0c60, 0x1de9, 0x2f72, 0x3efb, 0xd68d, 0xc704,
        0xf59f, 0xe416, 0x90a9, 0x8120, 0xb3bb, 0xa232, 0x5ac5, 0x4b4c, 0x79d7, 0x685e, 0x1ce1, 0x0d68, 0x3ff3, 0x2e7a,
        0xe70e, 0xf687, 0xc41c, 0xd595, 0xa12a, 0xb0a3, 0x8238, 0x93b1, 0x6b46, 0x7acf, 0x4854, 0x59dd, 0x2d62, 0x3ceb,
        0x0e70, 0x1ff9, 0xf78f, 0xe606, 0xd49d, 0xc514, 0xb1ab, 0xa022, 0x92b9, 0x8330, 0x7bc7, 0x6a4e, 0x58d5, 0x495c,
        0x3de3, 0x2c6a, 0x1ef1, 0x0f78};
    return (fcs >> 8) ^ fcs_table[(fcs ^ byte) & 0xff];
}
