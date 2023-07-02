//
// Created by Ward on 15/05/2023.
//

#ifndef JD_CPP_JDGPIOGAMEPAD_H
#define JD_CPP_JDGPIOGAMEPAD_H


#include "services/abstract/JdGamepad.h"

class JdGPIOGamepad : public JdGamepad {
public:
    // Creates a gamepad service, in particular an analog version.
    // if the gamepad is digital, set pinX to NO_PIN (other pin* don't matter)
    // variant is set according to the service specification. See gamepad.h
    // pinX/pinY are the wipers for the two potentiometers on the gamepad
    explicit JdGPIOGamepad(uint8_t pinX, uint8_t pinY, uint8_t variant = JD_GAMEPAD_VARIANT_THUMB,
                           uint32_t buttons_available = 0, uint8_t pinBtns[16] = nullptr, uint8_t pinLeds[16] = nullptr);


private:
    jd_gamepad_direction_t get_reading() override;

protected:
    void sensor_init() override;
private:
    bool activeHigh_;
    uint8_t pinL_ = 0,
            pinH_ = 0,
            pinX_,
            pinY_;
    uint8_t pinBtns_[16] = {0};
    uint8_t pinLeds_[16] = {0};
    uint32_t buttons_available_;
public:
    bool isActiveHigh() const;

    void setActiveHigh(bool activeHigh);

    // pinL is the lower reference voltage and pinH is the higher reference voltage (usually you only
    // want one)
    uint8_t getPinL() const;

    // pinL is the lower reference voltage and pinH is the higher reference voltage (usually you only
    // want one)
    void setPinL(uint8_t pinL);

    // pinL is the lower reference voltage and pinH is the higher reference voltage (usually you only
    // want one)
    uint8_t getPinH() const;

    // pinL is the lower reference voltage and pinH is the higher reference voltage (usually you only
    // want one)
    void setPinH(uint8_t pinH);
};


#endif //JD_CPP_JDGPIOGAMEPAD_H
