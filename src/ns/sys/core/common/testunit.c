#include "ns/include/error.h"
#include "ns/include/nstd.h"
#include "ns/sys/core/common/instance.h"
#include <string.h>
#include <stdio.h>

// MESSAGE TEST FUNCTIONS -----------------------------------------------------
// --- private functions declarations
static ns_error_t
verify_message_queue_content(message_queue_t *queue, int expected_length, ...);

#if 0
static ns_error_t
verify_priority_queue_content(priority_queue_t *queue, int expected_length, ...);
#endif

// --- message test functions
// --- private functions
static ns_error_t
verify_message_queue_content(message_queue_t *queue, int expected_length, ...)
{
    ns_error_t error = NS_ERROR_NONE;
    va_list args;
    message_t message;
    message_t msg_arg;

    va_start(args, expected_length);

    if (expected_length == 0) {
        message = message_queue_get_head(queue);
        if (message != NULL) {
            printf("message queue is not empty when expected length is zero.\r\n");
            EXIT_NOW(error = NS_ERROR_FAILED);
        }
    } else {
        for (message = message_queue_get_head(queue); message != NULL; message = message_get_next(message)) {
            if (expected_length == 0) {
                printf("message queue contains more entries than expected.\r\n");
                EXIT_NOW(error = NS_ERROR_FAILED);
            }

            msg_arg = va_arg(args, message_t);

            if (msg_arg != message) {
                printf("message queue content does not match what is expected.\r\n");
                EXIT_NOW(error = NS_ERROR_FAILED);
            }

            expected_length--;
        }

        if (expected_length != 0) {
            printf("message queue contains less entries than expected\r\n");
            EXIT_NOW(error = NS_ERROR_FAILED);
        }
    }

exit:
    va_end(args);
    return error;
}

#if 0
static ns_error_t
verify_priority_queue_content(priority_queue_t *queue, int expected_length, ...)
{
    return NS_ERROR_NONE;
}
#endif
// END OF MESSAGE TEST FUNCTIONS -----------------------------------------------
