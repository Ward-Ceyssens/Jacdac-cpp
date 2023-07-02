#include "JdBrailleDisplay.h"
extern "C" {
#include "jacdac-c/services/jd_services.h"
#include "jacdac-c/services/interfaces/jd_pins.h"
#include "jacdac-c/jacdac/dist/c/brailledisplay.h"

}
#define STATE_DIRTY 0x01






/*
From Unicode:
1 4
2 5
3 6
7 8
*/
static const uint8_t masks[] = {0x01, 0x02, 0x04, 0x40, 0x08, 0x10, 0x20, 0x80};

void JdBrailleDisplay::braile_display_send() {
    for (int col = 0; col < length_ * 2; ++col) {
        for (int row = 0; row < 4; ++row) {
            clear_all();

            uint32_t ch = get_channels(row, col);
            uint16_t ch0 = ch & 0xffff;
            uint16_t ch1 = (ch >> 16) & 0xffff;

            if (dots_[col >> 1] & masks[row + ((col & 1) ? 4 : 0)]) {
                channel_set(ch0, 1);
                channel_set(ch1, 0);
            } else {
                channel_set(ch0, 0);
                channel_set(ch1, 1);
            }

            write_channels();
            jd_services_sleep_us(7000);
            clear_all();
            write_channels();
            jd_services_sleep_us(4000);
        }
    }
}

void JdBrailleDisplay::handle_get() {
    int dst = 0;
    int len = length_;
    // don't send spaces at the end
    while (len > 0 && dots_[len - 1] == 0)
        len--;
    for (int i = 0; i < len; ++i) {
        uint8_t low = dots_[i];
        tmpbuf_[dst++] = 0xE2;
        tmpbuf_[dst++] = 0xA0 | (low >> 6);
        tmpbuf_[dst++] = 0x80 | (low & 0x3F);
    }
    jd_send(state_->service_index, JD_GET(JD_BRAILLE_DISPLAY_REG_PATTERNS), tmpbuf_, dst);
}

void JdBrailleDisplay::process() {
    if (flags_ & STATE_DIRTY) {
        flags_ &= ~STATE_DIRTY; // it's possible we'll get another set while we're updating;
                                      // we want that set to set the dirty flag again
        if (enabled_)
            braile_display_send();
        handle_get(); // make sure the UI refreshes
    }
}

void JdBrailleDisplay::handle_set(jd_packet_t *pkt) {
    int ptr = 0;
    for (int i = 0; i < pkt->service_size + 2; i += 3) {
        if (pkt->data[i] != 0xE2)
            break;
        if ((pkt->data[i + 1] & 0xFC) != 0xA0)
            break;
        if ((pkt->data[i + 2] & 0xC0) != 0x80)
            break;
        dots_[ptr++] = ((pkt->data[i + 1] & 3) << 6) | (pkt->data[i + 2] & 0x3f);
        if (ptr >= length_)
            break;
    }
    while (ptr < length_)
        dots_[ptr++] = 0;
    flags_ |= STATE_DIRTY;
}

void JdBrailleDisplay::handle_packet(jd_packet_t *pkt) {
    switch (pkt->service_command) {
        case JD_SET(JD_BRAILLE_DISPLAY_REG_PATTERNS):
                handle_set(pkt);
                break;

        case JD_GET(JD_BRAILLE_DISPLAY_REG_PATTERNS):
                handle_get();
                break;
            default:
                service_handle_register_final(pkt);
    }
}


JdBrailleDisplay::JdBrailleDisplay(uint8_t length) {
	addRegister(JD_BRAILLE_DISPLAY_REG_ENABLED, enabled_);
	addRegister(JD_BRAILLE_DISPLAY_REG_LENGTH, length_);
    

    enabled_ = 1;
    length_ = length;
    flags_ = 0;

    JD_ASSERT(length < DOTS_MAX);

    memset(dots_, 0, DOTS_MAX);

    braile_display_send();
    target_wait_us(3000);
}

void JdBrailleDisplay::initialize() {
    init();
    JacdacService::initialize();
}
