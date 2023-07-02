//
// Created by Ward on 15/05/2023.
//

#include "JdGPIOBuzzer.h"


#ifdef __cplusplus
extern "C" {
#endif
#include "jacdac-c/services/jd_services.h"
#include "jacdac-c/services/interfaces/jd_pins.h"
#include "jacdac-c/services/interfaces/jd_pwm.h"
#include "jacdac-c/services/interfaces/jd_hw_pwr.h"
#ifdef __cplusplus
}
#endif

#ifndef BUZZER_OFF
#define BUZZER_OFF 0
#endif
void JdGPIOBuzzer::set_pwr(bool on) {
    if (is_on_ == on)
        return;
    if (on) {
        pwr_enter_tim();
    } else {
        pin_set(pin_, BUZZER_OFF);
        jd_pwm_enable(pwm_pin_, 0);
        pwr_leave_tim();
    }
    is_on_ = on;
}

void JdGPIOBuzzer::play_tone(uint32_t period, uint32_t duty) {
    duty = (duty * getVolume()) >> 8;
#if BUZZER_OFF == 0
    duty = period - duty;
#endif
    set_pwr(1);
    pwm_pin_ = jd_pwm_init(pin_, period, duty, cpu_mhz);
}
JdGPIOBuzzer::JdGPIOBuzzer(uint8_t pin) : pin_(pin) {
    pin_set(pin_, BUZZER_OFF);
    pin_setup_output(pin_);
}