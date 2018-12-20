#include "ns/sys/core/mac/mac.h"
#include "ns/sys/core/common/code_utils.h"
#include "ns/sys/core/common/debug.h"
#include "ns/sys/core/common/encoding.h"
#include "ns/sys/core/common/instance.h"
#include "ns/sys/core/common/random.h"
// TODO: #include "ns/sys/core/crypto/aes_ccm.h"
// TODO: #include "ns/sys/core/crypto/sha256.h"
#include "ns/sys/core/mac/mac_frame.h"
#include "ns/sys/core/thread/link_quality.h"

static const uint8_t s_mode2_key[] = {0x78, 0x58, 0x16, 0x86, 0xfd, 0xb4, 0x58, 0x0f,
                                      0xb0, 0x92, 0x54, 0x6a, 0xec, 0xbd, 0x15, 0x66};

static const ns_error_t s_mode2_ext_addr = {
    {0x35, 0x06, 0xfe, 0xb8, 0x23, 0xd4, 0x87, 0x12},
};

static const ns_extended_panid_t s_extended_panid_init = {
    {0xde, 0xad, 0x00, 0xbe, 0xef, 0x00, 0xca, 0xfe},
};

static const char s_network_name_init[] = "ns-thread";

// --- private functions declarations
static void
mac_process_transmit_security(mac_t *mac, mac_frame_t *frame, bool process_aes_ccm);

static void
mac_generate_nonce(mac_t *mac,
                   const mac_ext_addr_t *addr,
                   uint32_t frame_counter,
                   uint8_t security_level,
                   uint8_t *nonce);

ns_error_t
mac_process_receive_security_frame(mac_t *mac,
                                   mac_frame_t *frame,
                                   const mac_addr_t *src_addr,
                                   neighbor_t *neighbor);
