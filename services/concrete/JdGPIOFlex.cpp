//
// Created by Ward on 17/05/2023.
//

#include "JdGPIOFlex.h"
extern "C" {
#include "jacdac-c/services/interfaces/jd_pins.h"
#include "jacdac-c/services/interfaces/jd_adc.h"
}
JdGPIOFlex::JdGPIOFlex(uint8_t pinL, uint8_t pinM, uint8_t pinH) {

   pinL_ = pinL;
   pinM_ = pinM;
   pinH_ = pinH;
}

uint16_t JdGPIOFlex::get_reading() {
    pin_setup_output(pinH_);
    pin_set(pinH_, 1);
    pin_setup_output(pinL_);
    pin_set(pinL_, 0);

    uint16_t result = adc_read_pin(pinM_);

    // save power
    pin_setup_analog_input(pinH_);
    pin_setup_analog_input(pinL_);

    return result;
}
