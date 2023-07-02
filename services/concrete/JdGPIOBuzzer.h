//
// Created by Ward on 15/05/2023.
//

#ifndef JD_CPP_JDGPIOBUZZER_H
#define JD_CPP_JDGPIOBUZZER_H


#include "services/abstract/JdBuzzer.h"

class JdGPIOBuzzer : public JdBuzzer {

    void set_pwr(bool on) override;

    uint8_t
            pwm_pin_ = 0;
    uint8_t
            pin_ = 0;

    void play_tone(uint32_t period, uint32_t duty) override;

    uint8_t
            is_on_ = 0;
public:
    explicit JdGPIOBuzzer(uint8_t pin);
};


#endif //JD_CPP_JDGPIOBUZZER_H
