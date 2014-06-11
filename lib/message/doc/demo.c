/**
 * \example demo.c
 * This example shows the basic usage of the SPADIC message library.
 *
 * An array of 16-bit values is created, which represents the data coming
 * from the chip. It contains two hit messages, where one word of the
 * second message is deliberately commented out, so that the raw data
 * containing the samples is invalid.
 *
 * The output should look like this:
 *
 *     message is hit
 *     group ID: 1
 *     channel ID: 2
 *     timestamp: 1638
 *     num. samples: 7
 *     hit type: 1
 *     stop type: 0
 *     samples: 1 2 3 4 5 6 7
 *
 *     message is hit
 *     group ID: 3
 *     channel ID: 4
 *     timestamp: 2184
 *     num. samples: 7
 *     hit type: 1
 *     stop type: 0
 *     raw data invalid
 */
#include <stdio.h>
#include "message.h"

#define N 15
const uint16_t buf[N] = {
    0x8012, /* start of message 1 */
    0x9666,
    0xA008,
    0x0403,
    0x0100,
    0x5030,
    0x0E00,
    0xB1D0, /* end of message 1 */
    0x8034, /* start of message 2 */
    0x9888,
    0xA008,
/*  0x0403, missing -> raw data invalid */
    0x0100,
    0x5030,
    0x0E00,
    0xB1D0, /* end of message 2 */
};

void print_hit_message(Message *m)
{
    printf("group ID: %d\n", message_get_group_id(m));
    printf("channel ID: %d\n", message_get_channel_id(m));
    printf("timestamp: %d\n", message_get_timestamp(m));
    printf("num. samples: %d\n", message_get_num_samples(m));
    printf("hit type: %d\n", message_get_hit_type(m));
    printf("stop type: %d\n", message_get_stop_type(m));
    int16_t *s;
    if (s = message_get_samples(m)) {
        printf("samples: ");
        int i = 0;
        while (i < message_get_num_samples(m)) {
            printf("%d ", s[i++]);
        }
        printf("\n");
    } else {
        printf("raw data invalid\n");
    }
}

int main(void)
{
    Message *m = message_new();
    size_t n = 0;
    while (n < N) {
        n += message_read_from_buffer(m, buf+n, N-n);
        if (message_is_hit(m)) {
            printf("\nmessage is hit\n");
            print_hit_message(m);
            message_reset(m);
        }
    }
    message_delete(m);
    return 0;
}
