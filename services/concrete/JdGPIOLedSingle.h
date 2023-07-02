//
// Created by Ward on 15/05/2023.
//

#ifndef JD_CPP_JDGPIOLEDSINGLE_H
#define JD_CPP_JDGPIOLEDSINGLE_H


#include "services/abstract/JdLedSingle.h"

class JdGPIOLedSingle : public JdLedSingle {

    uint8_t active_high_ = 0;
    uint16_t pwm_period_ = 512;

    void show() override;

    void update() override;
    void animate(const jd_control_set_status_light_t *anim) override;

    const color_t *getCurrentColor() override;
    typedef struct {
        uint16_t value;
        int16_t speed;
        uint8_t target;
        uint8_t pin;
        uint8_t pwm;
        uint8_t mult;
    } channel_t;

    uint8_t numch_ = 0;
    channel_t channels_[3];
    uint8_t flags_ = 0;

public:
    explicit JdGPIOLedSingle(uint8_t pin, uint8_t mult = 255);

    JdGPIOLedSingle(uint8_t pin_r, uint8_t pin_g, uint8_t pin_b, uint8_t mult = 255, uint16_t pwm_period = 512);

    JdGPIOLedSingle(uint8_t pin_r, uint8_t pin_g, uint8_t pin_b, uint8_t mult_r, uint8_t mult_g, uint8_t mult_b, uint16_t pwm_period = 512);

    uint8_t getActiveHigh() const;

    void setActiveHigh(uint8_t activeHigh);

    uint16_t getPwmPeriod() const;

    void setPwmPeriod(uint16_t pwmPeriod);
};


#endif //JD_CPP_JDGPIOLEDSINGLE_H
