//
// Created by Ward on 17/03/2023.
//

#ifndef JACDAC_COMPATIBILITY_JDBUTTON_H
#define JACDAC_COMPATIBILITY_JDBUTTON_H
#include "JdSensor.h"
#include "jacdac-c/jacdac/dist/c/button.h"

class JdButton : public JdSensor {
public:
    JdButton();
    void initialize() override;
    void process() override;
    void handle_packet(jd_packet_t *pkt) override;

    uint32_t getServiceClass() override {
        return JD_SERVICE_CLASS_BUTTON;
    }
protected:
    virtual bool getButtonPressed() = 0;

    virtual void setBacklight(bool on) = 0;
private:
    uint8_t
    pressed_ = 0
    ,
    prev_pressed_ = 0;

    uint32_t next_hold_ = 0,
    press_time_ = 0,
    nextSample_ = 0;

    void update();
};


#endif //JACDAC_COMPATIBILITY_JDBUTTON_H