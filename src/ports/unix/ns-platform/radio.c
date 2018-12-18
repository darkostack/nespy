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
get_dst_pan(const uint8_t *frame);

static ns_short_addr_t
get_short_addr(const uint8_t *frame);

static void
get_ext_addr(const uint8_t *frame, ns_ext_addr_t *addr);

static uint16_t
crc16_citt(uint16_t fcs, uint8_t byte);

static void
radio_compute_crc(struct _radio_message *message, uint16_t length);

static void
radio_transmit(struct _radio_message *message, const struct _ns_radio_frame *frame);

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

void
ns_plat_radio_set_extended_addr(ns_instance_t instance, const ns_ext_addr_t *ext_addr)
{
    (void)instance;
    for (size_t i = 0; i < sizeof(s_extended_addr); i++) {
        s_extended_addr[i] = ext_addr->m8[sizeof(s_extended_addr) - 1 - i];
    }
}

void
ns_plat_radio_set_short_addr(ns_instance_t instance, uint16_t addr)
{
    (void)instance;
    s_short_addr = addr;
}

void
ns_plat_radio_set_promiscuous(ns_instance_t instance, bool enabled)
{
    (void)instance;
    s_promiscuous = enabled;
}

void
plat_radio_init(void)
{
    struct sockaddr_in sockaddr;
    char *offset;
    memset(&sockaddr, 0, sizeof(sockaddr));
    sockaddr.sin_family = AF_INET;

    offset = getenv("PORT_OFFSET");

    if (offset) {
        char *endptr;
        s_port_offset = (uint16_t)strtol(offset, &endptr, 0);
        if (*endptr != '\0') {
            fprintf(stderr, "Invalid PORT_OFFSET: %s\n", offset);
            exit(EXIT_FAILURE);
        }
        s_port_offset *= WELLKNOWN_NODE_ID;
    }

    if (s_promiscuous) {
        sockaddr.sin_port = htons(9000 + s_port_offset + WELLKNOWN_NODE_ID);
    } else {
        sockaddr.sin_port = htons(9000 + s_port_offset + g_node_id);
    }

    sockaddr.sin_addr.s_addr = INADDR_ANY;

    s_sock_fd = (int)socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    if (s_sock_fd == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    if (bind(s_sock_fd, (struct sockaddr *)&sockaddr, sizeof(sockaddr)) == -1) {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    s_receive_frame.psdu = s_receive_message.psdu;
    s_transmit_frame.psdu = s_transmit_message.psdu;
    s_ack_frame.psdu = s_ack_message.psdu;

#if NS_CONFIG_HEADER_IE_SUPPORT
    s_transmit_frame.ie_info = &s_transmit_ie_info;
    s_receive_frame.ie_info = &s_received_ie_info;
#else
    s_transmit_frame.ie_info = NULL;
    s_receive_frame.ie_info = NULL;
#endif
}

void
plat_radio_deinit(void)
{
    close(s_sock_fd);
}

bool
ns_plat_radio_is_enabled(ns_instance_t instance)
{
    (void)instance;
    return (s_state != NS_RADIO_STATE_DISABLED) ? true : false;
}

ns_error_t
ns_plat_radio_enable(ns_instance_t instance)
{
    if (!ns_plat_radio_is_enabled(instance)) {
        s_state = NS_RADIO_STATE_SLEEP;
    }
    return NS_ERROR_NONE;
}

ns_error_t
ns_plat_radio_disable(ns_instance_t instance)
{
    if (ns_plat_radio_is_enabled(instance)) {
        s_state = NS_RADIO_STATE_DISABLED;
    }
    return NS_ERROR_NONE;
}

ns_error_t
ns_plat_radio_sleep(ns_instance_t instance)
{
    (void)instance;
    ns_error_t error = NS_ERROR_INVALID_STATE;
    if (s_state == NS_RADIO_STATE_SLEEP || s_state == NS_RADIO_STATE_RECEIVE) {
        error = NS_ERROR_NONE;
        s_state = NS_RADIO_STATE_SLEEP;
    }
    return error;
}

ns_error_t
ns_plat_radio_receive(ns_instance_t instance, uint8_t channel)
{
    (void)instance;
    ns_error_t error = NS_ERROR_INVALID_STATE;
    if (s_state != NS_RADIO_STATE_DISABLED) {
        error = NS_ERROR_NONE;
        s_state = NS_RADIO_STATE_RECEIVE;
        s_ack_wait = false;
        s_receive_frame.channel = channel;
    }
    return error;
}

ns_error_t
ns_plat_radio_transmit(ns_instance_t instance, ns_radio_frame_t *frame)
{
    (void)instance;
    (void)frame;
    ns_error_t error = NS_ERROR_INVALID_STATE;
    if (s_state == NS_RADIO_STATE_RECEIVE) {
        error = NS_ERROR_NONE;
        s_state = NS_RADIO_STATE_TRANSMIT;
    }
    return error;
}

ns_radio_frame_t *
ns_plat_radio_get_transmit_buffer(ns_instance_t instance)
{
    (void)instance;
    return &s_transmit_frame;
}

int8_t ns_plat_radio_get_rssi(ns_instance_t instance)
{
    (void)instance;

    int8_t rssi = UNIX_LOW_RSSI_SAMPLE;
    uint8_t channel = s_receive_frame.channel;
    uint32_t probability_threshold;

    EXPECT((NS_RADIO_CHANNEL_MIN <= channel) && channel <= (NS_RADIO_CHANNEL_MAX));

    // To emulate a simple interference model, we return either a high or
    // a low  RSSI value with a fixed probability per each channel. The
    // probability is increased per channel by a constant.

    probability_threshold = (channel - NS_RADIO_CHANNEL_MIN) * UNIX_HIGH_RSSI_PROB_INC_PER_CHANNEL;

    if ((ns_plat_random_get() & 0xffff) < (probability_threshold * 0xffff / 100)) {
        rssi = UNIX_HIGH_RSSI_SAMPLE;
    }

exit:
    return rssi;
}

ns_radio_caps_t
ns_plat_radio_get_caps(ns_instance_t instance)
{
    (void)instance;
    return NS_RADIO_CAPS_NONE;
}

bool
ns_plat_radio_get_promiscuous(ns_instance_t instance)
{
    (void)instance;
    return s_promiscuous;
}

void
radio_receive(ns_instance_t instance)
{
    bool is_ack;

    ssize_t rval = recvfrom(s_sock_fd, (char *)&s_receive_message, sizeof(s_receive_message), 0, NULL, NULL);

    if (rval < 0) {
        perror("recvfrom");
        exit(EXIT_FAILURE);
    }

    if (ns_plat_radio_get_promiscuous(instance)) {
        // timestamp
        s_receive_frame.info.rx_info.msec = ns_plat_alarm_milli_get_now();
        s_receive_frame.info.rx_info.usec = 0; // don't support microsecond timer for now.
    }

#if NS_CONFIG_ENABLE_TIME_SYNC
    s_receive_frame.ie_info->timestamp = ns_plat_time_get();
#endif

    s_receive_frame.length = (uint8_t)(rval - 1);

    is_ack = is_frame_type_ack(s_receive_frame.psdu);

    if (s_ack_wait && s_transmit_frame.channel == s_receive_message.channel && is_ack &&
        get_dsn(s_receive_frame.psdu) == get_dsn(s_transmit_frame.psdu)) {
        s_state = NS_RADIO_STATE_RECEIVE;
        s_ack_wait = false;
        ns_plat_radio_tx_done(instance, &s_transmit_frame, &s_receive_frame, NS_ERROR_NONE);
    } else if ((s_state == NS_RADIO_STATE_RECEIVE || s_state == NS_RADIO_STATE_TRANSMIT) &&
               (s_receive_frame.channel == s_receive_message.channel) && (!is_ack || s_promiscuous)) {
        radio_process_frame(instance);
    }
}

void
plat_radio_update_fd_set(fd_set *read_fd, fd_set *write_fd, int *max_fd)
{
    if (read_fd != NULL && (s_state != NS_RADIO_STATE_TRANSMIT || s_ack_wait)) {
        FD_SET(s_sock_fd, read_fd);
        if (max_fd != NULL && *max_fd < s_sock_fd) {
            *max_fd = s_sock_fd;
        }
    }
    if (write_fd != NULL && s_state == NS_RADIO_STATE_TRANSMIT && !s_ack_wait) {
        FD_SET(s_sock_fd, write_fd);
        if (max_fd != NULL && *max_fd < s_sock_fd) {
            *max_fd = s_sock_fd;
        }
    }
}

void
plat_radio_process(ns_instance_t instance)
{
    const int flags  = POLLIN | POLLRDNORM | POLLERR | POLLNVAL | POLLHUP;
    struct pollfd pollfd = {s_sock_fd, flags, 0};

    if (POLL(&pollfd, 1, 0) > 0 && (pollfd.revents & flags) != 0)
    {
        radio_receive(instance);
    }

    if (s_state == NS_RADIO_STATE_TRANSMIT && !s_ack_wait)
    {
        radio_send_message(instance);
    }
}

void
ns_plat_radio_enable_src_match(ns_instance_t instance, bool enable)
{
    (void)instance;
    s_src_match_enabled = enable;
}

ns_error_t
ns_plat_radio_add_src_match_short_entry(ns_instance_t instance, const uint16_t short_addr)
{
    (void)instance;
    ns_error_t error = NS_ERROR_NONE;
    EXPECT_ACTION(s_short_addr_match_table_count < sizeof(s_short_addr_match_table) / sizeof(uint16_t),
                  error = NS_ERROR_NO_BUFS);
    for (uint8_t i = 0; i < s_short_addr_match_table_count; ++i) {
        EXPECT_ACTION(s_short_addr_match_table[i] != short_addr, error = NS_ERROR_DUPLICATED);
    }
    s_short_addr_match_table[s_short_addr_match_table_count++] = short_addr;
exit:
    return error;
}

ns_error_t
ns_plat_radio_add_src_match_ext_entry(ns_instance_t instance, const ns_ext_addr_t *ext_addr)
{
    (void)instance;
    ns_error_t error = NS_ERROR_NONE;
    EXPECT_ACTION(s_ext_addr_match_table_count < sizeof(s_ext_addr_match_table) / sizeof(ns_ext_addr_t),
                  error = NS_ERROR_NO_BUFS);
    for (uint8_t i = 0; i < s_ext_addr_match_table_count; ++i) {
        EXPECT_ACTION(memcmp(&s_ext_addr_match_table[i], ext_addr, sizeof(ns_ext_addr_t)),
                      error = NS_ERROR_DUPLICATED);
    }
    s_ext_addr_match_table[s_ext_addr_match_table_count++] = *ext_addr;
exit:
    return error;
}

ns_error_t
ns_plat_radio_clear_src_match_short_entry(ns_instance_t instance, const uint16_t short_addr)
{
    (void)instance;
    ns_error_t error = NS_ERROR_NOT_FOUND;
    EXPECT(s_short_addr_match_table_count > 0);
    for (uint8_t i = 0; i < s_short_addr_match_table_count; ++i) {
        if (s_short_addr_match_table[i] == short_addr) {
            s_short_addr_match_table[i] = s_short_addr_match_table[--s_short_addr_match_table_count];
            error = NS_ERROR_NONE;
            goto exit;
        }
    }
exit:
    return error;
}

ns_error_t
ns_plat_radio_clear_src_match_ext_entry(ns_instance_t instance, const ns_ext_addr_t *ext_addr)
{
    (void)instance;
    ns_error_t error = NS_ERROR_NOT_FOUND;
    EXPECT(s_ext_addr_match_table_count > 0);
    for (uint8_t i = 0; i < s_ext_addr_match_table_count; ++i) {
        if (!memcmp(&s_ext_addr_match_table[i], ext_addr, sizeof(ns_ext_addr_t))) {
            s_ext_addr_match_table[i] = s_ext_addr_match_table[--s_ext_addr_match_table_count];
            error = NS_ERROR_NONE;
            goto exit;
        }
    }
exit:
    return error;
}

void
ns_plat_radio_clear_src_match_short_entries(ns_instance_t instance)
{
    (void)instance;
    s_short_addr_match_table_count = 0;
}

void
ns_plat_radio_clear_src_match_ext_entries(ns_instance_t instance)
{
    (void)instance;
    s_ext_addr_match_table_count = 0;
}

ns_error_t
ns_plat_radio_energy_scan(ns_instance_t instance, uint8_t scan_channel, uint16_t scan_duration)
{
    (void)instance;
    (void)scan_channel;
    (void)scan_duration;
    return NS_ERROR_NOT_IMPLEMENTED;
}

ns_error_t
ns_plat_radio_get_transmit_power(ns_instance_t instance, int8_t *power)
{
    (void)instance;
    *power = s_tx_power;
    return NS_ERROR_NONE;
}

ns_error_t
ns_plat_radio_set_transmit_power(ns_instance_t instance, int8_t power)
{
    (void)instance;
    s_tx_power = power;
    return NS_ERROR_NONE;
}

int8_t
ns_plat_radio_get_receive_sensitivity(ns_instance_t instance)
{
    (void)instance;
    return UNIX_RECEIVE_SENSITIVITY;
}

// --- private functions
static bool
find_short_address(uint16_t short_addr)
{
    uint8_t i;
    for (i = 0; i < s_short_addr_match_table_count; ++i) {
        if (s_short_addr_match_table[i] == short_addr) {
            break;
        }
    }
    return i < s_short_addr_match_table_count;
}

static bool
find_ext_address(const ns_ext_addr_t *ext_addr)
{
    uint8_t i;
    for (i = 0; i < s_ext_addr_match_table_count; ++i) {
        if (!memcmp(&s_ext_addr_match_table[i], ext_addr, sizeof(ns_ext_addr_t))) {
            break;
        }
    }
    return i < s_ext_addr_match_table_count;
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
        if (s_src_match_enabled) {
            has_frame_pending = find_short_address((uint16_t)(cur[1] << 8 | cur[0]));
        }
        cur += sizeof(ns_short_addr_t);
        break;
    case IEEE802154_SRC_ADDR_EXT:
        if (!is_panid_compressed(frame)) {
            cur += sizeof(ns_panid_t);
        }
        if (s_src_match_enabled) {
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
get_dst_pan(const uint8_t *frame)
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

static void
radio_compute_crc(struct _radio_message *message, uint16_t length)
{
    uint16_t crc = 0;
    uint16_t crc_offset = length - sizeof(uint16_t);
    for (uint16_t i = 0; i < crc_offset; i++) {
        crc = crc16_citt(crc, message->psdu[i]);
    }
    message->psdu[crc_offset] = crc & 0xff;
    message->psdu[crc_offset + 1] = crc >> 8;
}

static void
radio_transmit(struct _radio_message *message, const struct _ns_radio_frame *frame)
{
    uint32_t i;
    struct sockaddr_in sockaddr;

    if (!s_promiscuous) {
        radio_compute_crc(message, frame->length);
    }

    memset(&sockaddr, 0, sizeof(sockaddr));
    sockaddr.sin_family = AF_INET;
    inet_pton(AF_INET, "127.0.0.1", &sockaddr.sin_addr);

    for (i = 1; i <= WELLKNOWN_NODE_ID; i++) {
        ssize_t rval;
        if (g_node_id == i) {
            continue;
        }
        sockaddr.sin_port = htons(9000 + s_port_offset + i);
        rval = sendto(s_sock_fd, (const char *)message, 1 + frame->length, 0, (struct sockaddr *)&sockaddr,
                      sizeof(sockaddr));
        if (rval < 0) {
            perror("sendto");
            exit(EXIT_FAILURE);
        }
    }
}

void
radio_send_message(ns_instance_t instance)
{
#if NS_CONFIG_HEADER_IE_SUPPORT
    bool notify_frame_updated = false;
#if NS_CONFIG_ENABLE_TIME_SYNC
    if (s_transmit_frame.ie_info->time_ie_offset != 0) {
        uint8_t *time_ie = s_transmit_frame.psdu + s_transmit_frame.ie_info->time_ie_offset;
        uint64_t time = (uint64_t)((int64_t)ns_plat_time_get() + s_transmit_frame.ie_info->network_time_offset);
        *time_ie = s_transmit_frame.ie_info->time_sync_seq;
        *(++time_ie) = (uint8_t)(time & 0xff);
        for (uint8_t i = 1; i < sizeof(uint64_t); i++) {
            time = time >> 8;
            *(++time_ie) = (uint8_t)(time & 0xff);
        }
        notify_frame_updated = true;
    }
#endif // NS_CONFIG_ENABLE_TIME_SYNC
    if (notify_frame_updated) {
        ns_plat_radio_frame_updated(instance, &s_transmit_frame);
    }
#endif // NS_CONFIG_HEADER_IE_SUPPORT

    s_transmit_message.channel = s_transmit_frame.channel;

    ns_plat_radio_tx_started(instance, &s_transmit_frame);
    radio_transmit(&s_transmit_message, &s_transmit_frame);

    s_ack_wait = is_ack_requested(s_transmit_frame.psdu);

    if (!s_ack_wait) {
        s_state = NS_RADIO_STATE_RECEIVE;
        ns_plat_radio_tx_done(instance, &s_transmit_frame, NULL, NS_ERROR_NONE);
    }
}

static void
radio_send_ack(void)
{
    s_ack_frame.length = IEEE802154_ACK_LENGTH;
    s_ack_message.psdu[0] = IEEE802154_FRAME_TYPE_ACK;

    if (is_data_request_and_has_frame_pending(s_receive_frame.psdu)) {
        s_ack_message.psdu[0] |= IEEE802154_FRAME_PENDING;
    }

    s_ack_message.psdu[1] = 0;
    s_ack_message.psdu[2] = get_dsn(s_receive_frame.psdu);

    s_ack_message.channel = s_receive_frame.channel;

    radio_transmit(&s_ack_message, &s_ack_frame);
}

static void
radio_process_frame(ns_instance_t instance)
{
    ns_error_t error = NS_ERROR_NONE;
    ns_panid_t dstpan;
    ns_short_addr_t short_addr;
    ns_ext_addr_t ext_addr;

    EXPECT_ACTION(s_promiscuous == false, error = NS_ERROR_NONE);

    switch (s_receive_frame.psdu[1] & IEEE802154_DST_ADDR_MASK) {
    case IEEE802154_DST_ADDR_NONE:
        break;
    case IEEE802154_DST_ADDR_SHORT:
        dstpan = get_dst_pan(s_receive_frame.psdu);
        short_addr = get_short_addr(s_receive_frame.psdu);
        EXPECT_ACTION((dstpan == IEEE802154_BROADCAST || dstpan == s_panid) &&
                      (short_addr == IEEE802154_BROADCAST || short_addr == s_short_addr),
                      error = NS_ERROR_ABORT);
        break;
    case IEEE802154_DST_ADDR_EXT:
        dstpan = get_dst_pan(s_receive_frame.psdu);
        get_ext_addr(s_receive_frame.psdu, &ext_addr);
        EXPECT_ACTION((dstpan == IEEE802154_BROADCAST || dstpan == s_panid) &&
                      memcmp(&ext_addr, s_extended_addr, sizeof(ext_addr)) == 0,
                      error = NS_ERROR_ABORT);
        break;
    default:
        error = NS_ERROR_ABORT;
        goto exit;
    }

    s_receive_frame.info.rx_info.rssi = -20;
    s_receive_frame.info.rx_info.lqi = NS_RADIO_LQI_NONE;

    // generate acknowledgment
    if (is_ack_requested(s_receive_frame.psdu)) {
        radio_send_ack();
    }

exit:
    if (error != NS_ERROR_ABORT) {
        ns_plat_radio_receive_done(instance, error == NS_ERROR_NONE ? &s_receive_frame : NULL, error);
    }
}

__attribute__((weak)) void
ns_plat_radio_receive_done(ns_instance_t instance, ns_radio_frame_t *frame, ns_error_t error)
{
    (void)instance;
    (void)frame;
    (void)error;
}

__attribute__((weak)) void
ns_plat_diag_radio_receive_done(ns_instance_t instance, ns_radio_frame_t *frame, ns_error_t error)
{
    (void)instance;
    (void)frame;
    (void)error;
}

__attribute__((weak)) void
ns_plat_radio_tx_started(ns_instance_t instance, ns_radio_frame_t *frame)
{
    (void)instance;
    (void)frame;
}

__attribute__((weak)) void
ns_plat_radio_tx_done(ns_instance_t instance, ns_radio_frame_t *frame, ns_radio_frame_t *ack_frame, ns_error_t error)
{
    (void)instance;
    (void)frame;
    (void)ack_frame;
    (void)error;
}

__attribute__((weak)) void
ns_plat_diag_radio_transmit_done(ns_instance_t instance, ns_radio_frame_t *frame, ns_error_t error)
{
    (void)instance;
    (void)frame;
    (void)error;
}

__attribute__((weak)) void
ns_plat_radio_frame_updated(ns_instance_t instance, ns_radio_frame_t *frame)
{
    (void)instance;
    (void)frame;
}

__attribute__((weak)) void
ns_plat_radio_energy_scan_done(ns_instance_t instance, int8_t energy_scan_max_rssi)
{
    (void)instance;
    (void)energy_scan_max_rssi;
}
