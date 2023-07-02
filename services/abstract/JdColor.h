//
// generated on 2023-05-16.
// regenerate=0
//

#ifndef JD_CPP_COLOR_H
#define JD_CPP_COLOR_H

#include "JdSensor.h"
#include "jacdac-c/jacdac/dist/c/color.h"
#include "jacdac-c/services/interfaces/jd_sensor_api.h"

class JdColor : public JdSensor {
public:
    explicit JdColor();
    void process() override;
    void handle_packet(jd_packet_t *pkt) override;

    uint32_t getServiceClass() override {
        return JD_SERVICE_CLASS_COLOR;
    }


private:
    void *generic_sensor_get_reading() override;
    struct color_t{
        uint32_t r;
        uint32_t g;
        uint32_t b;
    };
    virtual const color_t* get_reading() = 0;


    uint16_t sample_[3]{0};
    uint32_t nextSample_ = 0;

    void update();
};


#endif //JD_CPP_COLOR_H