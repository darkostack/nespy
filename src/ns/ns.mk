# Network Stack core makefile

INC += -I$(TOP)/ns
INC += -I$(TOP)/ns/net
INC += -I$(TOP)/ns/net/ipv6
INC += -I$(TOP)/ns/services

SRC_NS_SYS += $(addprefix ns/sys/,\
    autostart.c \
    compower.c \
    ctimer.c \
    energest.c \
    etimer.c \
    log.c \
    mutex.c \
    node-id.c \
    process.c \
    rtimer.c \
    stack-check.c \
    stimer.c \
    timer.c \
    )

SRC_NS_LIB += $(addprefix ns/lib/,\
    aes-128.c \
    assert.c \
    ccm-star.c \
    circular-list.c \
    crc16.c \
    dbl-circ-list.c \
    dbl-list.c \
    heapmem.c \
    ifft.c \
    list.c \
    memb.c \
    ringbuf.c \
    ringbufindex.c \
    trickle-timer.c \
    )

SRC_NS_DEV += $(addprefix ns/dev/,\
    nullradio.c \
    serial-line.c \
    slip.c \
    )

SRC_NS_STORAGE_ANTELOPE += $(wildcard ns/storage/antelope/,*.c)
SRC_NS_STORAGE_CFS += $(wildcard ns/storage/cfs/,*.c)

SRC_NS_NET += $(addprefix ns/net/,\
    link-stats.c \
    linkaddr.c \
    nbr-table.c \
    net-debug.c \
    netstack.c \
    packetbuf.c \
    queuebuf.c \
    )

SRC_NS_NET_MAC += $(addprefix ns/net/mac/,\
    mac-sequence.c \
    mac.c \
    )

SRC_NS_NET_MAC_FRAMER += $(addprefix ns/net/mac/framer/,\
    frame802154.c \
    frame802154e-ie.c \
    framer-802154.c \
    nullframer.c \
    )

SRC_NS_NET_MAC_CSMA += $(addprefix ns/net/mac/csma/,\
    csma-output.c \
    csma.c \
    )

SRC_NS_NET_MAC_NULLMAC += $(addprefix ns/net/mac/nullmac/,\
    nullmac.c \
    )

SRC_NS_NET_MAC_BLE += $(addprefix ns/net/mac/ble/,\
    ble-l2cap.c \
    )

SRC_NS_NET_MAC_TSCH += $(addprefix ns/net/mac/tsch/,\
    tsch-adaptive-timesync.c \
    tsch-log.c \
    tsch-packet.c \
    tsch-queue.c \
    tsch-rpl.c \
    tsch-schedule.c \
    tsch-security.c \
    tsch-slot-operation.c \
    tsch-stats.c \
    tsch.c \
    )

SRC_NS_NET_MAC_TSCH_SIXTOP += $(addprefix ns/net/mac/tsch/sixtop/,\
    sixp-nbr.c \
    sixp-pkt.c \
    sixp-trans.c \
    sixp.c \
    sixtop.c \
    )

SRC_NS_NET_NULLNET += $(addprefix ns/net/nullnet/,\
    nullnet.c \
    )

SRC_NS_NET_ROUTING_NULLROUTING += $(addprefix ns/net/routing/nullrouting/,\
    nullrouting.c \
    )

SRC_NS_NET_ROUTING_RPL_CLASSIC += $(addprefix ns/net/routing/rpl-classic/,\
    rpl-dag-root.c \
    rpl-dag.c \
    rpl-ext-header.c \
    rpl-icmp6.c \
    rpl-mrhof.c \
    rpl-nbr-policy.c \
    rpl-of0.c \
    rpl-timers.c \
    rpl.c \
    )

SRC_NS_NET_ROUTING_RPL_LITE += $(addprefix ns/net/routing/rpl-lite/,\
    rpl-dag-root.c \
    rpl-dag.c \
    rpl-ext-header.c \
    rpl-icmp6.c \
    rpl-mrhof.c \
    rpl-nbr-policy.c \
    rpl-neighbor.c \
    rpl-of0.c \
    rpl-timers.c \
    rpl.c \
    )

SRC_NS_NET_IPV6 += $(addprefix ns/net/ipv6/,\
    ip64-addr.c \
    psock.c \
    resolv.c \
    sicslowpan.c \
    simple-udp.c \
    tcp-socket.c \
    tcpip.c \
    udp-socket.c \
    uip-ds6-nbr.c \
    uip-ds6-route.c \
    uip-ds6.c \
    uip-icmp6.c \
    uip-nameserver.c \
    uip-nd6.c \
    uip-packetqueue.c \
    uip-sr.c \
    uip-udp-packet.c \
    uip6.c \
    uipbuf.c \
    uiplib.c \
    )

SRC_NS_NET_IPV6_MULTICAST += $(addprefix ns/net/ipv6/multicast/,\
    esmrf.c \
    roll-tm.c \
    smrf.c \
    uip-mcast6-route.c \
    uip-mcast6-stats.c \
    )

SRC_NS_NET_APP_LAYER_COAP += $(addprefix ns/net/app-layer/coap/,\
    coap-block1.c \
    coap-blocking-api.c \
    coap-callback-api.c \
    coap-engine.c \
    coap-keystore-simple.c \
    coap-log.c \
    coap-observe-client.c \
    coap-observe.c \
    coap-res-well-known-core.c \
    coap-separate.c \
    coap-timer-default.c \
    coap-timer.c \
    coap-transactions.c \
    coap-uip.c \
    coap.c \
    )

SRC_NS_NET_APP_LAYER_COAP_TINYDTLS_SUPPORT += \
    $(addprefix ns/net/app-layer/coap/tinydtls-support/,\
    dtls-support.c \
    )

SRC_NS_NET_APP_LAYER_HTTP_SOCKET += \
    $(addprefix ns/net/app-layer/http-socket/,\
    http-socket.c \
    websocket-http-client.c \
    websocket.c \
    )

SRC_NS_NET_APP_LAYER_HTTPD_WS += \
    $(addprefix ns/net/app-layer/httpd-ws/,\
    httpd-ws.c \
    )

SRC_NS_NET_APP_LAYER_MQTT += \
    $(addprefix ns/net/app-layer/mqtt/,\
    mqtt.c \
    )

SRC_NS_SERVICES_IP64 += $(addprefix ns/services/ip64/,\
    ip64-addrmap.c \
    ip64-arp.c \
    ip64-dhcpc.c \
    ip64-dns64.c \
    ip64-eth-interface.c \
    ip64-eth.c \
    ip64-ipv4-dhcp.c \
    ip64-null-driver.c \
    ip64-slip-interface.c \
    ip64-special-ports.c \
    ip64.c \
    )

OBJ += $(addprefix $(BUILD)/,$(SRC_NS_SYS:.c=.o))
OBJ += $(addprefix $(BUILD)/,$(SRC_NS_DEV:.c=.o))
OBJ += $(addprefix $(BUILD)/,$(SRC_NS_LIB:.c=.o))
OBJ += $(addprefix $(BUILD)/,$(SRC_NS_NET:.c=.o))
OBJ += $(addprefix $(BUILD)/,$(SRC_NS_NET_MAC:.c=.o))
OBJ += $(addprefix $(BUILD)/,$(SRC_NS_NET_MAC_FRAMER:.c=.o))

# configure APP layer
MAKE_APP_COAP = 0
MAKE_APP_MQTT = 1

MAKE_APP ?= MAKE_APP_COAP

ifeq ($(MAKE_APP),MAKE_APP_COAP)
	OBJ += $(addprefix $(BUILD)/,$(SRC_NS_NET_APP_LAYER_COAP:.c=.o))
	CFLAGS += -DAPP_CONF_WITH_COAP=1
endif

ifeq ($(MAKE_APP),MAKE_APP_MQTT)
	OBJ += $(addprefix $(BUILD)/,$(SRC_NS_NET_APP_LAYER_MQTT:.c=.o))
	CFLAGS += -DAPP_CONF_WITH_MQTT=1
endif

# configure MAC layer

MAKE_MAC_NULLMAC = 0
MAKE_MAC_CSMA = 1
MAKE_MAC_TSCH = 2
MAKE_MAC_BLE = 3
MAKE_MAC_OTHER = 4

# Make CSMA the default MAC
MAKE_MAC ?= MAKE_MAC_CSMA

ifeq ($(MAKE_MAC),MAKE_MAC_NULLMAC)
  	OBJ += $(addprefix $(BUILD)/,$(SRC_NS_NET_MAC_NULLMAC:.c=.o))
  	CFLAGS += -DMAC_CONF_WITH_NULLMAC=1
endif

ifeq ($(MAKE_MAC),MAKE_MAC_CSMA)
  	OBJ += $(addprefix $(BUILD)/,$(SRC_NS_NET_MAC_CSMA:.c=.o))
  	CFLAGS += -DMAC_CONF_WITH_CSMA=1
endif

ifeq ($(MAKE_MAC),MAKE_MAC_TSCH)
  	OBJ += $(addprefix $(BUILD)/,$(SRC_NS_NET_MAC_TSCH:.c=.o))
  	CFLAGS += -DMAC_CONF_WITH_TSCH=1
endif

ifeq ($(MAKE_MAC),MAKE_MAC_BLE)
  	OBJ += $(addprefix $(BUILD)/,$(SRC_NS_NET_MAC_BLE:.c=.o))
  	CFLAGS += -DMAC_CONF_WITH_BLE=1
endif

ifeq ($(MAKE_MAC),MAKE_MAC_OTHER)
  	CFLAGS += -DMAC_CONF_WITH_OTHER=1
endif

# Configure Network layer

MAKE_NET_NULLNET = 0
MAKE_NET_IPV6 = 1
MAKE_NET_OTHER = 2

# Make IPv6 the default stack
MAKE_NET ?= MAKE_NET_IPV6

ifeq ($(MAKE_NET),MAKE_NET_NULLNET)
  	OBJ += $(addprefix $(BUILD)/,$(SRC_NS_NET_NULLNET:.c=.o))
  	CFLAGS += -DNETSTACK_CONF_WITH_NULLNET=1
endif

ifeq ($(MAKE_NET),MAKE_NET_IPV6)
  	OBJ += $(addprefix $(BUILD)/,$(SRC_NS_NET_IPV6:.c=.o))
  	CFLAGS += -DNETSTACK_CONF_WITH_IPV6=1
endif

ifeq ($(MAKE_NET),MAKE_NET_OTHER)
  	CFLAGS += -DNETSTACK_CONF_WITH_OTHER=1
endif

ifeq ($(WITH_IP64),1)
  	OBJ += $(addprefix $(BUILD)/,$(SRC_NS_SERVICES_IP64:.c=.o))
endif

# Configure Routing protocol
MAKE_ROUTING_NULLROUTING = 0
MAKE_ROUTING_RPL_CLASSIC = 1
MAKE_ROUTING_RPL_LITE = 2

# Default routing protocol: RPL for IPv6, None otherwise
ifeq ($(MAKE_NET),MAKE_NET_IPV6)
MAKE_ROUTING ?= MAKE_ROUTING_RPL_LITE
else
MAKE_ROUTING ?= MAKE_ROUTING_NULLROUTING
endif

ifeq ($(MAKE_ROUTING),MAKE_ROUTING_RPL_CLASSIC)
  	OBJ += $(addprefix $(BUILD)/,$(SRC_NS_NET_ROUTING_RPL_CLASSIC:.c=.o))
  	CFLAGS += -DROUTING_CONF_RPL_CLASSIC=1
else ifeq ($(MAKE_ROUTING),MAKE_ROUTING_RPL_LITE)
  	OBJ += $(addprefix $(BUILD)/,$(SRC_NS_NET_ROUTING_RPL_LITE:.c=.o))
  	CFLAGS += -DROUTING_CONF_RPL_LITE=1
else ifeq ($(MAKE_ROUTING),MAKE_ROUTING_NULLROUTING)
  	OBJ += $(addprefix $(BUILD)/,$(SRC_NS_NET_ROUTING_NULLROUTING:.c=.o))
  	CFLAGS += -DROUTING_CONF_NULLROUTING=1
endif

