#include "ns/contiki.h"
#include "ns/sys/node-id.h"
#include "ns/dev/radio.h"
#include "ns/net/packetbuf.h"
#include "ns/net/queuebuf.h"
#include "ns/net/netstack.h"
#include "ns/net/mac/csma/csma.h"
#include "ns/lib/py/nstd.h"
#include "port_unix.h"
#include <stdbool.h>

/* Log configuration */
#include "ns/sys/log.h"
#define LOG_MODULE "RADIO"
#define LOG_LEVEL LOG_LEVEL_RADIO

#define DEFAULT_PORT 9000
#define ACK_WAIT_TIME 100 // 100ms ack timeout

enum {
    WELLKNOWN_NODE_ID = 34,
    UNIX_RADIO_BUFFER_SIZE = 127,
};

typedef enum _radio_state_t {
    RADIO_STATE_DISABLED = 0,
    RADIO_STATE_SLEEP = 1,
    RADIO_STATE_RECEIVE = 2,
    RADIO_STATE_TRANSMIT = 3,
} radio_state_t;

PROCESS(unix_radio_thread, "Unix-radio process");
AUTOSTART_PROCESSES(&unix_radio_thread);

radio_value_t radio_tx_mode;
radio_value_t radio_rx_mode;
static uint8_t radio_channel;
static bool radio_on = false;
static bool radio_tx_broadcast = false;
static int8_t radio_rx_pkt_counter = 0;
static int8_t radio_txpower;
static uint8_t radio_tx_len;
static uint8_t radio_rx_len;
static uint16_t radio_port;
static bool radio_is_ack_received = false;
static radio_state_t radio_state = RADIO_STATE_DISABLED;
static bool radio_ack_wait = false;
static uint32_t ack_timeout;
static uint8_t radio_rx_buf[UNIX_RADIO_BUFFER_SIZE];
static uint8_t radio_tx_buf[UNIX_RADIO_BUFFER_SIZE];
static int sock_fd;

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
        int len = unix_read(packetbuf_dataptr(), PACKETBUF_SIZE);
        LOG_DBG("receive %d\r\n", len);
        packetbuf_set_datalen(len);
        NETSTACK_MAC.input();
        if (radio_rx_pkt_counter > 0) {
            radio_rx_pkt_counter--;
        }
    }

    PROCESS_END();
}

static int unix_init(void)
{
    unix_radio_init();

    radio_on = true;
    radio_channel = IEEE802154_DEFAULT_CHANNEL;
    radio_txpower = 20; // 20dBm
    radio_tx_mode = RADIO_TX_MODE_SEND_ON_CCA;
    radio_rx_mode = 0;

    radio_state = RADIO_STATE_RECEIVE;

    return 1;
}

static int unix_prepare(const void *data, unsigned short len)
{
    radio_tx_len = len;
    ns_memcpy((uint8_t *)&radio_tx_buf, (void *)data, MIN(len, UNIX_RADIO_BUFFER_SIZE));
    return RADIO_TX_OK;
}

static int unix_transmit(unsigned short len)
{
    // check radio tx len
    if (radio_tx_len != len) {
        LOG_ERR("transmit length mismatch\r\n");
        return RADIO_TX_ERR;
    }

    if (radio_tx_len < CSMA_ACK_LEN) {
        LOG_ERR("invalid transmit length, min (%d)\r\n", CSMA_ACK_LEN);
        return RADIO_TX_ERR;
    }

    // check channel statuc if CCA mode was set and we are not send an ack
    if ((radio_tx_mode & RADIO_TX_MODE_SEND_ON_CCA) && (radio_tx_len > CSMA_ACK_LEN)) {
        if (!unix_channel_clear()) {
            LOG_WARN("transmit collision\r\n");
            return RADIO_TX_COLLISION;
        }
    }

    radio_state = RADIO_STATE_TRANSMIT;

    if (radio_tx_len > CSMA_ACK_LEN) {
        radio_tx_broadcast = packetbuf_holds_broadcast() == 1 ? true: false;
    }

    // wait until transmit process is finished
    while (radio_state == RADIO_STATE_TRANSMIT) {
        unix_process_update();
    }

    // reset transmit broadcast flag
    if (radio_tx_broadcast) {
        radio_tx_broadcast = false;
    }

    return RADIO_TX_OK;
}

static int unix_send(const void *data, unsigned short len)
{
    unix_prepare(data, len);
    return unix_transmit(len);
}

static int unix_read(void *buf, unsigned short size)
{
    int ret;
    ns_memcpy((void *)buf, (uint8_t *)&radio_rx_buf, MIN(size, radio_rx_len));
    ret = radio_rx_len;
    if (radio_rx_len == CSMA_ACK_LEN) {
        radio_is_ack_received = false;
    }
    radio_rx_len = 0;
    return ret;
}

static int unix_channel_clear(void)
{
    return (radio_ack_wait || (radio_state == RADIO_STATE_TRANSMIT)) ? 0 : 1;
}

static int unix_receiving_packet(void)
{
    return 0;
}

static int unix_pending_packet(void)
{
    return (radio_rx_pkt_counter != 0 || radio_is_ack_received) ? 1 : 0;
}

static int unix_on(void)
{
    radio_state = RADIO_STATE_RECEIVE;
    return 1;
}

static int unix_off(void)
{
    radio_state = RADIO_STATE_DISABLED;
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
    radio_port = DEFAULT_PORT + node_id;
    sockaddr.sin_family = AF_INET;
    sockaddr.sin_port = htons(radio_port);
    sockaddr.sin_addr.s_addr = INADDR_ANY;

    sock_fd = (int)socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    if (sock_fd == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    if (bind(sock_fd, (struct sockaddr *)&sockaddr, sizeof(sockaddr)) == -1) {
        perror("bind");
        exit(EXIT_FAILURE);
    }
}

void unix_radio_update_fd_set(fd_set *read_fd_set, fd_set *write_fd_set, int *max_fd)
{
    if (read_fd_set != NULL && (radio_state != RADIO_STATE_TRANSMIT || radio_ack_wait)) {
        FD_SET(sock_fd, read_fd_set);
        if (max_fd != NULL && *max_fd < sock_fd) {
            *max_fd = sock_fd;
        }
    }
    if (write_fd_set != NULL && radio_state == RADIO_STATE_TRANSMIT && !radio_ack_wait) {
        FD_SET(sock_fd, write_fd_set);
        if (max_fd != NULL && *max_fd < sock_fd) {
            *max_fd = sock_fd;
        }
    }
}

void unix_radio_process(void)
{
    const int flags = POLLIN | POLLRDNORM | POLLERR | POLLNVAL | POLLHUP;
    struct pollfd pollfd = {sock_fd, flags, 0};

    if (POLL(&pollfd, 1, 0) > 0 && (pollfd.revents & flags) != 0) {
        unix_radio_update();
    }

    // check for ack
    if ((radio_rx_len == CSMA_ACK_LEN) && radio_ack_wait) {
        radio_is_ack_received = true;
    }

    // transmit done and acked 
    if (radio_is_ack_received && radio_ack_wait && (radio_state == RADIO_STATE_TRANSMIT) && 
        (ack_timeout >= RTIMER_NOW())) {
        radio_ack_wait = false;
        radio_state = RADIO_STATE_RECEIVE;
        LOG_DBG("transmit done, acked\r\n");
    }

    // transmit done but noack
    if (!radio_is_ack_received && radio_ack_wait && (radio_state == RADIO_STATE_TRANSMIT) &&
        (ack_timeout < RTIMER_NOW())) {
        radio_ack_wait = false;
        radio_state = RADIO_STATE_RECEIVE;
        LOG_WARN("transmit done, noack -- timeout: %d, now: %d\r\n", ack_timeout, (int)RTIMER_NOW());
    }

    // get a packet call mac input
    if (radio_rx_len > CSMA_ACK_LEN && (radio_state == RADIO_STATE_RECEIVE) && !radio_is_ack_received) {
        radio_rx_pkt_counter++;
        process_poll(&unix_radio_thread);
    }

    if (radio_state == RADIO_STATE_TRANSMIT && !radio_ack_wait) {
        unix_radio_transmit((uint8_t *)&radio_tx_buf, radio_tx_len);
        if (radio_tx_len > CSMA_ACK_LEN) {
            if (radio_tx_broadcast) {
                radio_state = RADIO_STATE_RECEIVE;
                LOG_DBG("transmit %d -- broadcast\r\n", radio_tx_len);
            } else {
                // not broadcast packet and need an ack
                radio_ack_wait = true;
                ack_timeout = RTIMER_NOW() + ACK_WAIT_TIME;
                LOG_DBG("transmit %d -- !broadcast\r\n", radio_tx_len);
            }
        } else {
            // we just sent an ack
            LOG_DBG("transmit -- ack\r\n");
            radio_state = RADIO_STATE_RECEIVE;
        }
    }
}

static void unix_radio_update(void)
{
    ssize_t rval = recvfrom(sock_fd, (char *)&radio_rx_buf, sizeof(radio_rx_buf), 0, NULL, NULL);
    if (rval < 0) {
        perror("recvfrom");
        exit(EXIT_FAILURE);
    }
    radio_rx_len = (uint8_t)rval;
    LOG_DBG("unix radio update recv (%d)\r\n", radio_rx_len);
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
        sockaddr.sin_port = htons(DEFAULT_PORT + i);
        rval = sendto(sock_fd, (const char *)buf, len, 0, (struct sockaddr *)&sockaddr,
                      sizeof(sockaddr));
        if (rval < 0) {
            perror("sendto");
            exit(EXIT_FAILURE);
        }
    }
}

uint16_t unix_radio_get_port(void)
{
    return radio_port;
}
