#include "ns/error.h"
#include "core/common/instance.h"
#include "core/common/tlvs.h"
#include "core/meshcop/meshcop_tlvs.h"
#include "test_util.h"
#include <string.h>
#include <stdio.h>

#define PRINT_DEBUG 0

#if PRINT_DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

ns_error_t
test_meshcop_panid_tlv(void)
{
    ns_error_t error = NS_ERROR_NONE;

    printf("------------------------- TEST MESHCOP PANID\r\n");

    meshcop_panid_tlv_t panid_tlv;

    meshcop_panid_tlv_init(&panid_tlv);

    VERIFY_OR_EXIT(meshcop_panid_tlv_is_valid(&panid_tlv));

    PRINTF("panid tlv length: %d, from total: %lu\r\n", tlv_get_length(&panid_tlv.tlv), sizeof(panid_tlv));

    meshcop_panid_tlv_set_panid(&panid_tlv, 0x1234);

    PRINTF("panid: 0x%04x\r\n", meshcop_panid_tlv_get_panid(&panid_tlv));

exit:
    if (error != NS_ERROR_NONE) {
        printf("FAILED\r\n");
    } else {
        printf("PASSED\r\n");
    }
    return error;
}
