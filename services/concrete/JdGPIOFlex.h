//
// Created by Ward on 17/05/2023.
//

#ifndef JD_CPP_JDGPIOFLEX_H
#define JD_CPP_JDGPIOFLEX_H


#include "services/abstract/JdFlex.h"

class JdGPIOFlex : public JdFlex {

    uint8_t pinH_ = 0;
    uint8_t pinL_ = 0;
    uint8_t pinM_ = 0;

    uint16_t get_reading() override;

public:
    explicit JdGPIOFlex(uint8_t pinL, uint8_t pinM, uint8_t pinH);
};


#endif //JD_CPP_JDGPIOFLEX_H
