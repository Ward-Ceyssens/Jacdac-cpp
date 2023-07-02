//
// Created by Ward on 15/05/2023.
//

#include "JdGPIOLedSingle.h"
extern "C"{
#include "jacdac-c/services/interfaces/jd_pins.h"
#include "jacdac-c/services/interfaces/jd_pwm.h"
#include "jacdac-c/services/interfaces/jd_hw_pwr.h"
}
#define RGB_IN_TIM 0x01

JdGPIOLedSingle::JdGPIOLedSingle(uint8_t pin, uint8_t mult) : JdGPIOLedSingle(pin, NO_PIN, NO_PIN, mult){}

JdGPIOLedSingle::JdGPIOLedSingle(uint8_t pin_r, uint8_t pin_g, uint8_t pin_b, uint8_t mult, uint16_t pwm_period) :
        JdGPIOLedSingle(pin_r, pin_g, pin_b, mult, mult, mult, pwm_period){}

JdGPIOLedSingle::JdGPIOLedSingle(uint8_t pin_r, uint8_t pin_g, uint8_t pin_b, uint8_t mult_r, uint8_t mult_g, uint8_t mult_b, uint16_t pwm_period) {
    channels_[0].pin = pin_r;
    channels_[0].mult = mult_r;

    channels_[1].pin = pin_g;
    channels_[1].mult = mult_g;

    channels_[2].pin = pin_b;
    channels_[2].mult = mult_b;

    numch_ = pin_g == NO_PIN ? 1 : 3;

    if (pwm_period < 512)
        pwm_period = 512;
    pwm_period_ = pwm_period;

    for (int i = 0; i < numch_; ++i) {
        channel_t *ch = &channels_[i];
        if (!ch->mult)
            ch->mult = 0xff;
        // assuming 64MHz clock and 512 period, we get 125kHz (which is slow enough for motor
        // drivers like DRV8837C) with 8MHz, we get 15kHz (which is plenty fast not to flicker)
        ch->pwm = jd_pwm_init(ch->pin, pwm_period_, 0, 1);
    }

    JdGPIOLedSingle::show();
}

uint8_t JdGPIOLedSingle::getActiveHigh() const {
    return active_high_;
}

void JdGPIOLedSingle::setActiveHigh(uint8_t activeHigh) {
    active_high_ = activeHigh;
}

uint16_t JdGPIOLedSingle::getPwmPeriod() const {
    return pwm_period_;
}

void JdGPIOLedSingle::setPwmPeriod(uint16_t pwmPeriod) {
    pwm_period_ = pwmPeriod;
}

void JdGPIOLedSingle::show() {
    int sum = 0;

    int prevR = channels_[0].value;

    if (numch_ == 1) {
        // take double green to avoid division when computing mean
        int tmp =
                channels_[0].value + 2 * channels_[1].value + channels_[2].value;
        channels_[0].value = tmp >> 2;
    }

    for (int i = 0; i < numch_; ++i) {
        channel_t *ch = &channels_[i];
        int32_t c = ch->value;
        c = (c * ch->mult) >> (7 + 8);
        sum += c;
        if (c == 0) {
            pin_set(ch->pin, JdGPIOLedSingle::active_high_ ? 0 : 1);
            jd_pwm_enable(ch->pwm, false);
        } else {
            if (JdGPIOLedSingle::active_high_) {
                if (c >= JdGPIOLedSingle::pwm_period_)
                    c = JdGPIOLedSingle::pwm_period_ - 1;
            } else {
                c = JdGPIOLedSingle::pwm_period_ - c;
                if (c < 0)
                    c = 0;
            }
            jd_pwm_set_duty(ch->pwm, c);
            jd_pwm_enable(ch->pwm, true);
        }
    }

    channels_[0].value = prevR;

    if (sum == 0 && (flags_ & RGB_IN_TIM)) {
        pwr_leave_tim();
        flags_ &= ~RGB_IN_TIM;
    } else if (sum != 0 && !(flags_ & RGB_IN_TIM)) {
        pwr_enter_tim();
        flags_ |= RGB_IN_TIM;
    }
}

void JdGPIOLedSingle::update() {
    int chg = 0;

    for (auto & channel : channels_) {
        channel_t *ch = &channel;
        int v0 = ch->value;
        int v = v0 + ch->speed;
        if (ch->speed == 0 ||                           // immediate
            (ch->speed < 0 && (v >> 8) < ch->target) || // undershoot
            (ch->speed > 0 && (v >> 8) > ch->target)    // overshoot
                ) {
            ch->speed = 0;
            ch->value = ch->target << 8;
        } else {
            ch->value = v;
        }
        if (v0 != ch->value)
            chg = 1;
    }

    if (chg)
        show();
}

const JdLedSingle::color_t *JdGPIOLedSingle::getCurrentColor() {
    return new color_t{static_cast<uint8_t>(channels_[0].value >> 8),
                       static_cast<uint8_t>(channels_[1].value >> 8),
                       static_cast<uint8_t>(channels_[2].value >> 8)};
}

void JdGPIOLedSingle::animate(const jd_control_set_status_light_t *anim) {
    const uint8_t *to = &anim->to_red;
    for (int i = 0; i < 3; ++i) {
        channel_t *ch = &channels_[i];
        ch->target = to[i];
        ch->speed = ((to[i] - (ch->value >> 8)) * anim->speed) >> 1;
    }
}
