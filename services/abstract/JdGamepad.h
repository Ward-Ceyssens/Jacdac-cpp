//
// generated on 2023-05-01.
// regenerate=false
//

#ifndef JD_CPP_GAMEPAD_H
#define JD_CPP_GAMEPAD_H

#include "JdSensor.h"
#include "jacdac-c/services/jd_services.h"
#include "jacdac-c/jacdac/dist/c/gamepad.h"

class JdGamepad : public JdSensor {
public:
    explicit JdGamepad(uint32_t buttons_available, uint8_t variant);
    void process() override;
    void handle_packet(jd_packet_t *pkt) override;

    uint32_t getServiceClass() override {
        return JD_SERVICE_CLASS_GAMEPAD;
    }
    

private:
    virtual jd_gamepad_direction_t get_reading() = 0;

    uint32_t buttons_available_;
    uint8_t variant_;
    jd_gamepad_direction_t direction_{};
    uint32_t nextSample_ = 0;

    void update();
	void maybe_init();
};


#endif //JD_CPP_GAMEPAD_H