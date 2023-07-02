//
// Created by Ward on 15/05/2023.
//

#ifndef JD_CPP_JDGPIOBUTTON_H
#define JD_CPP_JDGPIOBUTTON_H


#include "services/abstract/JdButton.h"

class JdGPIOButton : public JdButton {
public:
    explicit JdGPIOButton(uint8_t pin, bool active = false, uint8_t backlight_pin = NO_PIN);

private:
    uint8_t
            pin_ = 0;
    uint8_t
            backlight_pin_ = 0;
    uint8_t
            active_ = 0;

protected:
    bool getButtonPressed() override;

    void setBacklight(bool on) override;
};


#endif //JD_CPP_JDGPIOBUTTON_H
