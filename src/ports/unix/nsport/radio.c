#include "ns/contiki.h"
#include "ns/sys/node-id.h"
#include "ns/dev/radio.h"
#include "ns/net/packetbuf.h"
#include "ns/net/queuebuf.h"
#include "ns/net/netstack.h"
#include "ns/net/mac/csma/csma.h"
#include "ns/modules/nstd.h"
#include "port_unix.h"
#include <stdbool.h>

#define DEFAULT_PORT 9000
#define ACK_WAIT_TIME 100 // 100ms ack timeout

enum {
    WELLKNOWN_NODE_ID = 34,
    UNIX_RADIO_BUFFER_SIZE = 128,
};

typedef enum _radio_state_t {
    RADIO_STATE_DISABLED = 0,
    RADIO_STATE_SLEEP = 1,
    RADIO_STATE_RECEIVE = 2,
    RADIO_STATE_TRANSMIT = 3,
} radio_state_t;

PROCESS(unix_radio_thread, "Unix-radio process");
AUTOSTART_PROCESSES(&unix_radio_thread);

radio_value_t   s_radio_tx_mode;
radio_value_t   s_radio_rx_mode;
static uint8_t  s_radio_channel;
static int8_t   s_radio_rssi;
static bool     s_radio_on = false;
static bool     s_radio_tx_broadcast = false;
static uint32_t s_radio_tx_dsn;
static int8_t   s_radio_rx_pkt_counter = 0;
static int8_t   s_radio_txpower;
static uint8_t  s_radio_tx_len;
void *          s_radio_sent_packet_ptr;
static uint8_t  s_radio_rx_len;

static bool     s_ack_wait = false;
static uint32_t s_ack_timeout;

// static radio buffer allocation
static uint8_t  s_radio_rx_buf[UNIX_RADIO_BUFFER_SIZE];
static uint8_t  s_radio_tx_buf[UNIX_RADIO_BUFFER_SIZE];

static radio_state_t s_state = RADIO_STATE_DISABLED;
static uint16_t      s_port_offset = 0;
static int           s_sock_fd;

static void unix_radio_init(void);
static void unix_radio_update(void);
static void unix_radio_transmit(uint8_t *buf, uint8_t len);

// contiki radio api
static int unix_init(void);
static int unix_prepare(const void *data, unsigned short len);
static int unix_transmit(unsigned short len);
static int unix_send(const void *data, unsigned short len);
static int unix_read(void *buf, unsigned short size);
static int unix_channel_clear(void);
static int unix_receiving_packet(void);
static int unix_pending_packet(void);
static int unix_on(void);
static int unix_off(void);
static radio_result_t unix_get_value(radio_param_t param, radio_value_t *value);
static radio_result_t unix_set_value(radio_param_t param, radio_value_t value);
static radio_result_t unix_get_object(radio_param_t param, void *dest, size_t size);
static radio_result_t unix_set_object(radio_param_t param, const void *src, size_t size);

// extern from ns/net/mac/csma/csma-output.c
extern void packet_sent(void *ptr, int status, int num_transmissions);

const struct radio_driver unix_radio_driver = {
    unix_init,
    unix_prepare,
    unix_transmit,
    unix_send,
    unix_read,
    unix_channel_clear,
    unix_receiving_packet,
    unix_pending_packet,
    unix_on,
    unix_off,
    unix_get_value,
    unix_set_value,
    unix_get_object,
    unix_set_object
};

PROCESS_THREAD(unix_radio_thread, ev, data)
{
    PROCESS_BEGIN();

    while (1) {
        PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_POLL);
        packetbuf_clear();
        int read_len = unix_read(packetbuf_dataptr(), PACKETBUF_SIZE);
        packetbuf_set_datalen(read_len);
        s_radio_rssi = -10; // hardcoded rssi value
        packetbuf_set_attr(PACKETBUF_ATTR_CHANNEL, s_radio_channel);
        packetbuf_set_attr(PACKETBUF_ATTR_RSSI, s_radio_rssi);
        NETSTACK_MAC.input();
        if (s_radio_rx_pkt_counter > 0) {
            s_radio_rx_pkt_counter--;
        }
    }

    PROCESS_END();
}

static int unix_init(void)
{
    unix_radio_init();

    s_radio_on = true;
    s_radio_channel = IEEE802154_DEFAULT_CHANNEL;
    s_radio_txpower = 20; // 20dBm
    s_radio_tx_mode = RADIO_TX_MODE_SEND_ON_CCA;
    s_radio_rx_mode = 0;

    s_state = RADIO_STATE_RECEIVE;

    return 1;
}

static int unix_prepare(const void *data, unsigned short len)
{
    s_radio_tx_len = len;
    ns_memcpy((uint8_t *)&s_radio_tx_buf, (void *)data, MIN(len, UNIX_RADIO_BUFFER_SIZE));
    return RADIO_TX_OK;
}

static int unix_transmit(unsigned short len)
{
    if (s_radio_tx_len != len) return RADIO_TX_ERR;

    if ((s_radio_tx_mode & RADIO_TX_MODE_SEND_ON_CCA) && (s_radio_tx_len > CSMA_ACK_LEN)) {
        if (!unix_channel_clear()) {
            return RADIO_TX_COLLISION;
        }
    }

    s_state = RADIO_STATE_TRANSMIT;

    return RADIO_TX_OK;
}

static int unix_send(const void *data, unsigned short len)
{
    unix_prepare(data, len);
    return unix_transmit(len);
}

static int unix_read(void *buf, unsigned short size)
{
    ns_memcpy((uint8_t *)&s_radio_rx_buf, (void *)buf, MIN(size, UNIX_RADIO_BUFFER_SIZE));
    return size;
}

static int unix_channel_clear(void)
{
    return (s_ack_wait || (s_state == RADIO_STATE_TRANSMIT)) ? 0 : 1;
}

static int unix_receiving_packet(void)
{
    return 0;
}

static int unix_pending_packet(void)
{
    return (s_radio_rx_pkt_counter != 0) ? 1 : 0;
}

static int unix_on(void)
{
    s_state = RADIO_STATE_RECEIVE;
    return 1;
}

static int unix_off(void)
{
    s_state = RADIO_STATE_DISABLED;
    return 1;
}

static radio_result_t unix_get_value(radio_param_t param, radio_value_t *value)
{
    return RADIO_RESULT_NOT_SUPPORTED;
}

static radio_result_t unix_set_value(radio_param_t param, radio_value_t value)
{
    return RADIO_RESULT_NOT_SUPPORTED;
}

static radio_result_t unix_get_object(radio_param_t param, void *dest, size_t size)
{
    return RADIO_RESULT_NOT_SUPPORTED;
}

static radio_result_t unix_set_object(radio_param_t param, const void *src, size_t size)
{
    return RADIO_RESULT_NOT_SUPPORTED;
}

static void unix_radio_init(void)
{
    struct sockaddr_in sockaddr;
    memset(&sockaddr, 0, sizeof(sockaddr));
    char *offset;

    offset = getenv("PORT_OFFSET");
    sockaddr.sin_family = AF_INET;

    if (offset)
    {
        char *endptr;
        s_port_offset = (uint16_t)strtol(offset, &endptr, 0);
        s_port_offset *= WELLKNOWN_NODE_ID;
    }

    sockaddr.sin_port = htons(DEFAULT_PORT + s_port_offset + node_id);
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
}

void unix_radio_update_fd_set(fd_set *read_fd_set, fd_set *write_fd_set, int *max_fd)
{
    if (read_fd_set != NULL && (s_state != RADIO_STATE_TRANSMIT || s_ack_wait)) {
        FD_SET(s_sock_fd, read_fd_set);
        if (max_fd != NULL && *max_fd < s_sock_fd) {
            *max_fd = s_sock_fd;
        }
    }
    if (write_fd_set != NULL && s_state == RADIO_STATE_TRANSMIT && !s_ack_wait) {
        FD_SET(s_sock_fd, write_fd_set);
        if (max_fd != NULL && *max_fd < s_sock_fd) {
            *max_fd = s_sock_fd;
        }
    }
}

void unix_radio_process(void)
{
    const int     flags  = POLLIN | POLLRDNORM | POLLERR | POLLNVAL | POLLHUP;
    struct pollfd pollfd = {s_sock_fd, flags, 0};

    if (POLL(&pollfd, 1, 0) > 0 && (pollfd.revents & flags) != 0) {
        unix_radio_update();
    }

    if (s_state == RADIO_STATE_TRANSMIT && !s_ack_wait) {
        unix_radio_transmit((uint8_t *)&s_radio_tx_buf, s_radio_tx_len);
        if (s_radio_tx_len > CSMA_ACK_LEN) {
            if (s_radio_tx_broadcast) {
                s_state = RADIO_STATE_RECEIVE;
                packet_sent(s_radio_sent_packet_ptr, MAC_TX_OK, 1);
            } else {
                // not broadcast packet and need an ack
                s_ack_wait = true;
                s_ack_timeout = RTIMER_NOW() + ACK_WAIT_TIME;
            }
        } else {
            // we just sent an ack
            s_state = RADIO_STATE_RECEIVE;
        }
    }
}

static void unix_radio_update(void)
{
    bool is_ack_received = false;

    ssize_t rval = recvfrom(s_sock_fd, (char *)&s_radio_rx_buf, sizeof(s_radio_rx_buf), 0, NULL, NULL);

    if (rval < 0) {
        perror("recvfrom");
        exit(EXIT_FAILURE);
    }

    s_radio_rx_len = (uint8_t)rval;

    // check whether ack is valid
    if ((s_radio_rx_len == CSMA_ACK_LEN) && s_ack_wait) {
        uint8_t ack_buf[CSMA_ACK_LEN];
        unix_read(ack_buf, CSMA_ACK_LEN);
        if (ack_buf[2] == s_radio_tx_dsn) {
            // we received an ack!
            is_ack_received = true;
        }
    }

    // transmit ok and received valid ack
    if (is_ack_received && s_ack_wait && (s_state == RADIO_STATE_TRANSMIT) && 
        (s_ack_timeout <= RTIMER_NOW())) {
        s_ack_wait = false;
        s_state = RADIO_STATE_RECEIVE;
        packet_sent(s_radio_sent_packet_ptr, MAC_TX_OK, 1);
        goto exit;
    }

    // transmit ok but can't received ack
    if (!is_ack_received && s_ack_wait && (s_state == RADIO_STATE_TRANSMIT) &&
        (s_ack_timeout > RTIMER_NOW())) {
        s_ack_wait = false;
        s_state = RADIO_STATE_RECEIVE;
        packet_sent(s_radio_sent_packet_ptr, MAC_TX_NOACK, 1);
        goto exit;
    }

    // call mac input
    if ((s_state == RADIO_STATE_RECEIVE) && !is_ack_received) {
        s_radio_rx_pkt_counter++;
        process_post(&unix_radio_thread, PROCESS_EVENT_POLL, NULL);
    }

exit:
    return;
}

static void unix_radio_transmit(uint8_t *buf, uint8_t len)
{
    uint32_t i;
    struct sockaddr_in sockaddr;

    memset(&sockaddr, 0, sizeof(sockaddr));
    sockaddr.sin_family = AF_INET;
    inet_pton(AF_INET, "127.0.0.1", &sockaddr.sin_addr);

    for (i = 0; i <= WELLKNOWN_NODE_ID; i++) {
        ssize_t rval;
        if (node_id == i) {
            continue;
        }
        sockaddr.sin_port = htons(DEFAULT_PORT + s_port_offset + i);
        rval = sendto(s_sock_fd, (const char *)buf, len, 0, (struct sockaddr *)&sockaddr,
                      sizeof(sockaddr));
        if (rval < 0) {
            perror("sendto");
            exit(EXIT_FAILURE);
        }
    }
}

void send_one_packet(void *ptr)
{
    int ret;
    if (s_state != RADIO_STATE_TRANSMIT && !s_ack_wait) {
        s_radio_sent_packet_ptr = ptr;
        packetbuf_set_addr(PACKETBUF_ADDR_SENDER, &linkaddr_node_addr);
        packetbuf_set_attr(PACKETBUF_ATTR_MAC_ACK, 1);
        if (NETSTACK_FRAMER.create() < 0) {
            // failed to allocate space for headers
            ret = MAC_TX_ERR_FATAL;
        } else {
            s_radio_tx_dsn = ((uint8_t *)packetbuf_hdrptr())[2] & 0xff;
            unix_prepare(packetbuf_hdrptr(), packetbuf_totlen());
            s_radio_tx_broadcast = packetbuf_holds_broadcast() == 1 ? true: false;
            if (unix_receiving_packet() ||
                (!s_radio_tx_broadcast && unix_pending_packet())) {
                ret = MAC_TX_COLLISION;
            } else {
                switch (unix_transmit(packetbuf_totlen())) {
                case RADIO_TX_OK:
                    ret = MAC_TX_DEFERRED;
                    break;
                case RADIO_TX_COLLISION:
                    ret = MAC_TX_COLLISION;
                    break;
                default:
                    ret = MAC_TX_ERR;
                }
            }
        }
    } else {
        ret = MAC_TX_COLLISION;
    }

    if (ret != MAC_TX_DEFERRED) {
        packet_sent(ptr, ret, 1);
    }
}
