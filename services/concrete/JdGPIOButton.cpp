//
// Created by Ward on 15/05/2023.
//

#include "JdGPIOButton.h"
#include "jacdac-c/services/interfaces/jd_pins.h"
JdGPIOButton::JdGPIOButton(uint8_t pin, bool active, uint8_t backlight_pin) {
    pin_ = pin;
    active_ = active;
    backlight_pin_ = backlight_pin;
    pin_setup_output(backlight_pin_);
    pin_setup_input(pin_, active_ == 0 ? PIN_PULL_UP : PIN_PULL_DOWN);
}

bool JdGPIOButton::getButtonPressed() {
    return pin_get(pin_) == active_;
}

void JdGPIOButton::setBacklight(bool on) {
    pin_set(pin_, on);
}
