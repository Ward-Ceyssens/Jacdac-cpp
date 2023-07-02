//
// Created by Ward on 15/05/2023.
//

#include "JdGPIOGamepad.h"
#include "jacdac-c/services/interfaces/jd_pins.h"
extern "C" {
#include "jacdac-c/services/interfaces/jd_adc.h"
}
#define THRESHOLD_SWITCH 0x3000
#define THRESHOLD_KEEP 0x2000


JdGPIOGamepad::JdGPIOGamepad(uint8_t pinX, uint8_t pinY, uint8_t variant, uint32_t buttons_available,
                             uint8_t pinBtns[16], uint8_t pinLeds[16])
                             : JdGamepad(buttons_available, variant), pinX_(pinX), pinY_(pinY){
    if (pinBtns)
        memcpy(pinBtns_, pinBtns, sizeof(uint8_t)*16);

    if (pinLeds)
        memcpy(pinLeds_, pinLeds, sizeof(uint8_t)*16);

    for (unsigned i = 0; i < sizeof(pinBtns_); ++i) {
        if ((1 << i) & buttons_available_) {
            if (pinBtns_[i] == NO_PIN) {
                // not connected? clear the flag
                buttons_available_ &= ~(1 << i);
            }
        }
    }
}

void JdGPIOGamepad::sensor_init() {
    for (unsigned i = 0; i < sizeof(pinBtns_); ++i) {
        if ((1 << i) & buttons_available_) {
            pin_setup_input(pinBtns_[i],
                            activeHigh_ ? PIN_PULL_DOWN : PIN_PULL_UP);
            pin_setup_output(pinLeds_[i]);
        }
    }
    JdSensor::sensor_init();
}

jd_gamepad_direction_t JdGPIOGamepad::get_reading() {
    jd_gamepad_direction_t direction_{};
    uint32_t btns0 = direction_.buttons;
    uint32_t btns = 0;

    for (unsigned i = 0; i < sizeof(pinBtns_); ++i) {
        if ((1 << i) & buttons_available_) {
            if (pin_get(pinBtns_[i]) == activeHigh_) {
                btns |= 1 << i;
            }
        }
    }

    if (pinX_ == 0xff) {
        direction_.x = (btns & JD_GAMEPAD_BUTTONS_LEFT)
                       ? -0x8000
                       : (btns & JD_GAMEPAD_BUTTONS_RIGHT) ? 0x7fff : 0;
        direction_.y = (btns & JD_GAMEPAD_BUTTONS_UP)
                       ? -0x8000
                       : (btns & JD_GAMEPAD_BUTTONS_DOWN) ? 0x7fff : 0;
    } else {
        pin_setup_output(pinH_);
        pin_set(pinH_, 1);
        pin_setup_output(pinL_);
        pin_set(pinL_, 0);

        int x = adc_read_pin(pinX_) - 0x8000;
        int y = adc_read_pin(pinY_) - 0x8000;

        // save power
        pin_setup_analog_input(pinH_);
        pin_setup_analog_input(pinL_);

        direction_.x = x;
        direction_.y = y;

        if (direction_.x <
            ((btns0 & JD_GAMEPAD_BUTTONS_LEFT) ? -THRESHOLD_KEEP : -THRESHOLD_SWITCH))
            btns |= JD_GAMEPAD_BUTTONS_LEFT;
        if (direction_.x >
            ((btns0 & JD_GAMEPAD_BUTTONS_RIGHT) ? THRESHOLD_KEEP : THRESHOLD_SWITCH))
            btns |= JD_GAMEPAD_BUTTONS_RIGHT;
        if (direction_.y <
            ((btns0 & JD_GAMEPAD_BUTTONS_UP) ? -THRESHOLD_KEEP : -THRESHOLD_SWITCH))
            btns |= JD_GAMEPAD_BUTTONS_UP;
        if (direction_.y >
            ((btns0 & JD_GAMEPAD_BUTTONS_DOWN) ? THRESHOLD_KEEP : THRESHOLD_SWITCH))
            btns |= JD_GAMEPAD_BUTTONS_DOWN;
    }

    if (btns0 != btns) {
        direction_.buttons = btns;
        jd_send_event_ext((srv_t*)state_, JD_GAMEPAD_EV_BUTTONS_CHANGED, &direction_.buttons, 4);

        for (unsigned i = 0; i < sizeof(pinLeds_); ++i) {
            if ((1 << i) & buttons_available_) {
                pin_set(pinLeds_[i], btns & (1 << i) ? 0 : 1);
            }
        }
    }
    return direction_;
}

bool JdGPIOGamepad::isActiveHigh() const {
    return activeHigh_;
}

void JdGPIOGamepad::setActiveHigh(bool activeHigh) {
    activeHigh_ = activeHigh;
}

uint8_t JdGPIOGamepad::getPinL() const {
    return pinL_;
}

void JdGPIOGamepad::setPinL(uint8_t pinL) {
    pinL_ = pinL;
}

uint8_t JdGPIOGamepad::getPinH() const {
    return pinH_;
}

void JdGPIOGamepad::setPinH(uint8_t pinH) {
    pinH_ = pinH;
}


