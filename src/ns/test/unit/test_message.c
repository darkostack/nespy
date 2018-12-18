#include "ns/include/error.h"
#include "ns/sys/core/common/instance.h"
#include "ns/test/unit/test_util.h"
#include <string.h>
#include <stdio.h>

ns_error_t
test_message_write_read(void *instance)
{
    ns_error_t error = NS_ERROR_NONE;
    message_pool_t *message_pool = &((instance_t *)instance)->message_pool;

    printf("-------------------- TEST MESSAGE WRITE READ\r\n");

    uint8_t write_buffer[1024];
    uint8_t read_buffer[1024];

    extern uint32_t ns_plat_random_get(void);

    for (unsigned i = 0; i < sizeof(write_buffer); i++) {
        write_buffer[i] = (uint8_t)ns_plat_random_get();
    }

    message_t message = message_new(0, 0, 0);

    TEST_VERIFY_OR_EXIT(message != NULL, "failed to create the message.\r\n");
    TEST_VERIFY_OR_EXIT(message_set_length(message, sizeof(write_buffer)) == NS_ERROR_NONE,
                        "message set length failed.\r\n");

    TEST_VERIFY_OR_EXIT(message_write(message, 0, write_buffer, sizeof(write_buffer)) == sizeof(write_buffer),
                        "message write return length not as expected.\r\n");
    TEST_VERIFY_OR_EXIT(message_read(message, 0, read_buffer, sizeof(read_buffer)) == sizeof(read_buffer),
                        "message read return length not as expected.\r\n");
    TEST_VERIFY_OR_EXIT(memcmp(write_buffer, read_buffer, sizeof(write_buffer)) == 0,
                        "message write and read not identical as expected.\r\n");

    message_free(message);

    TEST_VERIFY_OR_EXIT(message_pool->num_free_buffers == MSG_NUM_BUFFERS,
                        "num of free buffers did not match as expected.\r\n");
exit:
    if (error != NS_ERROR_NONE) {
        printf("FAILED\r\n");
    } else {
        printf("PASSED\r\n");
    }

    return error;
}

ns_error_t
test_message_utility(void *instance)
{
    ns_error_t error = NS_ERROR_NONE;
    message_pool_t *message_pool = &((instance_t *)instance)->message_pool;

    uint8_t ref_buffer[128];
    uint8_t msgref_buffer[sizeof(ref_buffer)];
    uint8_t msgcopy_buffer[sizeof(ref_buffer)];
    uint8_t msgclone_buffer[sizeof(ref_buffer)];

    printf("----------------------- TEST MESSAGE UTILITY\r\n");

    message_t msgref = message_new(0, 0, 0);
    TEST_VERIFY_OR_EXIT(msgref != NULL, "message new failed.\r\n");

    message_t msgcopy = message_new(0, 0, 0);
    TEST_VERIFY_OR_EXIT(msgcopy != NULL, "message new failed.\r\n");

    extern uint32_t ns_plat_random_get(void);

    for (unsigned i = 0; i < sizeof(ref_buffer); i++) {
        ref_buffer[i] = (uint8_t)ns_plat_random_get();
    }

    TEST_VERIFY_OR_EXIT(message_set_length(msgref, sizeof(ref_buffer)) == NS_ERROR_NONE,
                        "message set length failed.\r\n");

    TEST_VERIFY_OR_EXIT(message_set_length(msgcopy, message_get_length(msgref)) == NS_ERROR_NONE,
                        "message set length failed.\r\n");

    TEST_VERIFY_OR_EXIT(message_write(msgref, 0, ref_buffer, sizeof(ref_buffer)) == sizeof(ref_buffer),
                        "message write length return not as expected.\r\n");

    // test message copy to
    TEST_VERIFY_OR_EXIT(message_copy_to(msgref, 0, 0, message_get_length(msgref), msgcopy) ==
                        message_get_length(msgref),
                        "message copy to length return not as expected.\r\n");

    TEST_VERIFY_OR_EXIT(message_read(msgref, 0, msgref_buffer, sizeof(msgref_buffer)) == sizeof(msgref_buffer),
                        "message read length return not as expected.\r\n");
    TEST_VERIFY_OR_EXIT(message_read(msgcopy, 0, msgcopy_buffer, sizeof(msgcopy_buffer)) == sizeof(msgcopy_buffer),
                        "message read length return not as expected.\r\n");

    TEST_VERIFY_OR_EXIT(memcmp(msgref_buffer, msgcopy_buffer, sizeof(msgref_buffer)) == 0,
                        "message copied not identical with reference as expected.\r\n");

    // --- free message copy message
    message_free(msgcopy);

    for (unsigned i = 0; i < sizeof(ref_buffer); i++) {
        ref_buffer[i] = (uint8_t)ns_plat_random_get();
    }

    // --- re-write msgref data with new random buffer
    TEST_VERIFY_OR_EXIT(message_write(msgref, 0, ref_buffer, sizeof(ref_buffer)) == sizeof(ref_buffer),
                        "message write length return not as expected.\r\n");

    message_t msgclone = message_clone(msgref);

    TEST_VERIFY_OR_EXIT(msgclone != NULL,
                        "message clone failed, it's return NULL.\r\n");

    TEST_VERIFY_OR_EXIT(message_read(msgref, 0, msgref_buffer, sizeof(msgref_buffer)) == sizeof(msgref_buffer),
                        "message read length return not as expected.\r\n");
    TEST_VERIFY_OR_EXIT(message_read(msgclone, 0, msgclone_buffer, sizeof(msgclone_buffer)) == sizeof(msgclone_buffer),
                        "message read length return not as expected.\r\n");

    TEST_VERIFY_OR_EXIT(memcmp(msgref_buffer, msgclone_buffer, sizeof(msgref_buffer)) == 0,
                        "message clone not identical with reference as expected.\r\n");

    // TODO: test message prepend, append and remove header function

    message_free(msgref);
    message_free(msgclone);

    TEST_VERIFY_OR_EXIT(message_pool->num_free_buffers == MSG_NUM_BUFFERS,
                        "num of free buffers did not match as what is expected.\r\n");
exit:
    if (error != NS_ERROR_NONE) {
        printf("FAILED\r\n");
    } else {
        printf("PASSED\r\n");
    }

    return error;
}
