#include <stdlib.h>

#include "message.h"

// move here again when cleaned up
#include "message_helper.h"

// temp
#include <stdio.h>

//===================================================================
// private functions
//===================================================================

int has_preamble(uint16_t w, struct Preamble p)
{
    return (w & p.mask) == p.value;
}

//-------------------------------------------------------------------

void message_init(Message* m)
{
    if (!m) return;
    if (m->data != NULL) {
        free(m->data);
        m->data = NULL;
    }
    m->valid = 0;
}

//-------------------------------------------------------------------

static int infotype_has_channel_id(uint8_t info_type)
{
    switch (info_type) {
        case 0: return 1; // iDIS
        case 1: return 1; // iNGT
        case 3: return 1; // iNBE
        case 4: return 1; // iMSB
    }
    return 0; // TODO use enum
}

//===================================================================
// public functions
//===================================================================

Message* message_new(void)
{
    Message* m;
    m = malloc(sizeof *m);
    message_init(m);
    return m;
}

//-------------------------------------------------------------------

void message_delete(Message* m)
{
    free(m->data);
    free(m);
}

//-------------------------------------------------------------------

size_t message_read_from_buffer(Message* m, uint16_t* buf, size_t len)
{
    Message _m;
    uint16_t* w;
    for (w=buf; w<buf+end; w++) {
        printf("word: %04X\n", *w);

        // start of message -> group ID, channel ID
        if (has_preamble(*w, wSOM)) {
            _m.group_id = (*w & 0x0FF0) >> 4;
            _m.channel_id = (*w & 0x000F);
            _m.valid |= 0x01;

        // timestamp
        } else if (has_preamble(*w, wTSW)) {
            _m.timestamp = (*w & 0x0FFF);
            _m.valid |= 0x02;

        // data...

        // end of message -> num. data, hit type, stop type
        } else if (has_preamble(*w, wEOM)) {
            _m.num_data = (*w & 0x0FC0) >> 6;
            _m.hit_type = (*w & 0x0030) >> 4;
            _m.stop_type = (*w & 0x0007);
            _m.valid |= 0x08;

        // buffer overflow count
        } else if (has_preamble(*w, wBOM)) {
            _m.buffer_overflow_count = (*w & 0x00FF);
            _m.valid |= 0x10;

        // epoch marker
        } else if (has_preamble(*w, wEPM)) {
            _m.epoch_count = (*w & 0x0FFF);
            _m.valid |= 0x20;

        // info words
        } else if (has_preamble(*w, wINF)) {
            _m.info_type = (*w & 0x0F00) >> 8;
            if any(has_preamble(*w, t])
                   for it in ['iDIS', 'iNGT', 'iNBE', 'iMSB']):
                _m.channel_id = (w & 0x00F0) >> 4
            } else if ( has_preamble(*w, iSYN)) {
                _m.epoch_count = (w & 0x00FF)
        }
    }
    return w+1;
}

//-------------------------------------------------------------------

int message_is_complete(Message* m)
{
    return (message_is_hit(m) ||
            message_is_buffer_overflow(m) ||
            message_is_epoch_marker(m) ||
            message_is_info(m));
//        m->valid == 0x0F || /* normal hit message */
//        m->valid == 0x30 || /* info message */
//     /* m->valid == ... ||                        */
//    0);
}

//===================================================================
// test/temp/dummy/wrap
//===================================================================

int seek_message_start_all(uint16_t* begin, uint16_t* end)
{
    Message m;
    int count = 0;
    uint16_t* pw = begin;
    while (pw<end) {
        printf("pw: %u\n", pw);
        pw = read_message(pw, end, &m);
        count++;
    }
    return count;
}

//-------------------------------------------------------------------

int seek_message_start_all_wrap(uint16_t* begin, unsigned int length)
{
    return seek_message_start_all(begin, begin+length);
}
