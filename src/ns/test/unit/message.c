#include "ns/include/error.h"
#include "ns/include/nstd.h"
#include "ns/sys/core/common/instance.h"
#include <string.h>
#include <stdio.h>

ns_error_t
test_message_write_read(void)
{
    ns_error_t error = NS_ERROR_NONE;
    instance_t *inst = instance_get();

    printf("-------------------- TEST MESSAGE WRITE READ\r\n");

    uint8_t write_buffer[1024];
    uint8_t read_buffer[1024];

    extern uint32_t ns_plat_random_get(void);

    for (unsigned i = 0; i < sizeof(write_buffer); i++) {
        write_buffer[i] = (uint8_t)ns_plat_random_get();
    }

    message_t message = message_new(0, 0, 0);

    if (message == NULL) {
        printf("failed to create the message!\r\n");
        EXIT_NOW();
    }

    error = message_set_length(message, sizeof(write_buffer));

    VERIFY_OR_EXIT(error == NS_ERROR_NONE);

    message_write(message, 0, write_buffer, sizeof(write_buffer));
    message_read(message, 0, read_buffer, sizeof(read_buffer));

    if (memcmp(write_buffer, read_buffer, sizeof(write_buffer)) == 0) {
        printf("message write and read test SUCCESS\r\n");
    } else {
        printf("message write and read test FAILED\r\n");
    }

    printf("num of free buffers: %u\r\n", inst->message_pool.num_free_buffers);

    message_free(message);

    printf("freed message, num of free buffers now: %u\r\n", inst->message_pool.num_free_buffers);

    if (inst->message_pool.num_free_buffers != MSG_NUM_BUFFERS) {
        printf("ERROR: invalid number of free buffers, expect %u\r\n", MSG_NUM_BUFFERS);
    }

exit:
    return error;
}

ns_error_t
test_message_utility(void)
{
    ns_error_t error = NS_ERROR_NONE;
    instance_t *inst = instance_get();
    uint8_t ref_buffer[128];
    uint8_t msgref_buffer[sizeof(ref_buffer)];
    uint8_t msgcopy_buffer[sizeof(ref_buffer)];

    printf("------------------------ TEST MESSAGE UTILITY\r\n");

    message_t msgref = message_new(0, 0, 0);
    VERIFY_OR_EXIT(msgref != NULL, error = NS_ERROR_NO_BUFS);

    message_t msgcopy = message_new(0, 0, 0);
    VERIFY_OR_EXIT(msgcopy != NULL, error = NS_ERROR_NO_BUFS);

    extern uint32_t ns_plat_random_get(void);

    for (unsigned i = 0; i < sizeof(ref_buffer); i++) {
        ref_buffer[i] = (uint8_t)ns_plat_random_get();
    }

    error = message_set_length(msgref, sizeof(ref_buffer));
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);

    error = message_set_length(msgcopy, message_get_length(msgref));
    VERIFY_OR_EXIT(error == NS_ERROR_NONE);

    message_write(msgref, 0, ref_buffer, sizeof(ref_buffer));

    // test message copy to
    message_copy_to(msgref, 0, 0, message_get_length(msgref), msgcopy);

    message_read(msgref, 0, msgref_buffer, sizeof(msgref_buffer));
    message_read(msgcopy, 0, msgcopy_buffer, sizeof(msgcopy_buffer));

    if (memcmp(msgref_buffer, msgcopy_buffer, sizeof(msgref_buffer)) == 0) {
        printf("message copy to SUCCESS\r\n");
    } else {
        printf("message copy to FAILED\r\n");
    }

    printf("num of free buffers: %u\r\n", inst->message_pool.num_free_buffers);

    // --- free message copy message
    message_free(msgcopy);

    printf("freed message, num of free buffers now: %u\r\n", inst->message_pool.num_free_buffers);

    // test message clone
    uint8_t msgclone_buffer[sizeof(ref_buffer)];

    for (unsigned i = 0; i < sizeof(ref_buffer); i++) {
        ref_buffer[i] = (uint8_t)ns_plat_random_get();
    }

    // --- re-write msgref data with new random buffer
    message_write(msgref, 0, ref_buffer, sizeof(ref_buffer));

    message_t msgclone = message_clone(msgref);

    message_read(msgref, 0, msgref_buffer, sizeof(msgref_buffer));
    message_read(msgclone, 0, msgclone_buffer, sizeof(msgclone_buffer));

    if (memcmp(msgref_buffer, msgclone_buffer, sizeof(msgref_buffer)) == 0) {
        printf("message clone SUCCESS\r\n");
    } else {
        printf("message clone FAILED\r\n");
    }

    printf("num of free buffers: %u\r\n", inst->message_pool.num_free_buffers);

    message_free(msgref);
    message_free(msgclone);

    printf("freed message, num of free buffers now: %u\r\n", inst->message_pool.num_free_buffers);

    if (inst->message_pool.num_free_buffers != MSG_NUM_BUFFERS) {
        printf("ERROR: invalid number of free buffers, expect %u\r\n", MSG_NUM_BUFFERS);
    }

    // TODO: test message prepend, append and remove header function

exit:
    return error;
}
