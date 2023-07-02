//
// Created by Ward on 17/03/2023.
//

#include "JdButton.h"
extern "C" {
#include "jacdac-c/services/jd_services.h"
}
#include "jacdac-c/jacdac/dist/c/button.h"



void JdButton::update() {

    pressed_ = getButtonPressed();
    if (pressed_ != prev_pressed_) {
        prev_pressed_ = pressed_;
        setBacklight(pressed_);

        if (pressed_) {
            jd_send_event(reinterpret_cast<srv_t *>(state_), JD_BUTTON_EV_DOWN);
            press_time_ = now;
            next_hold_ = 500000;
        } else {
            uint32_t presslen = (now - press_time_) / 1000;
            jd_send_event_ext(reinterpret_cast<srv_t *>(state_), JD_BUTTON_EV_UP, &presslen, sizeof(uint32_t));
        }
    }

    if (pressed_) {

        uint32_t presslen = now - press_time_;
        if (presslen >= next_hold_) {
            next_hold_ += 500000;
            presslen = presslen / 1000;
            jd_send_event_ext(reinterpret_cast<srv_t *>(state_), JD_BUTTON_EV_HOLD, &presslen, sizeof(uint32_t));
        }
    }
}
void JdButton::process() {
    if (jd_should_sample(&nextSample_, 9000)) {
        update();
    }
    uint16_t pressed = (pressed_ ? 0xffff : 0);
    sensor_process_simple(&pressed, sizeof(pressed));
}

void JdButton::handle_packet(jd_packet_t *pkt) {
    uint16_t pressed = (pressed_ ? 0xffff : 0);
    sensor_handle_packet_simple(pkt, &pressed, sizeof(pressed));

}
void JdButton::initialize() {
    JdSensor::initialize();
    update();
}

JdButton::JdButton() {

}

