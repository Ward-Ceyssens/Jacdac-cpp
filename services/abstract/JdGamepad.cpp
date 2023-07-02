#include "JdGamepad.h"
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
extern "C" {
#include "jacdac-c/services/jd_services.h"

#include "jacdac-c/jacdac/dist/c/gamepad.h"
}




void JdGamepad::update() {
    direction_ = get_reading();
}
void JdGamepad::maybe_init() {
    if (sensor_maybe_init()) {
        update();
    }
}
void JdGamepad::process() {
    maybe_init();

    if (jd_should_sample(&nextSample_, 9000) && jd_inited_)
        update();

    sensor_process_simple(&direction_, sizeof(direction_));
}
void JdGamepad::handle_packet(jd_packet_t *pkt) {
    if (service_handle_register(pkt))
        return;
    sensor_handle_packet_simple(pkt, &direction_, sizeof(direction_));
}


JdGamepad::JdGamepad(uint32_t buttons_available, uint8_t variant): buttons_available_(buttons_available), variant_(variant) {
    addRegister(JD_GAMEPAD_REG_BUTTONS_AVAILABLE, buttons_available_);
    addRegister(JD_GAMEPAD_REG_VARIANT, variant_);

}