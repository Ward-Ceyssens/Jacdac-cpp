//
// generated on 2023-05-17.
// regenerate=0
//

#ifndef JD_CPP_GYROSCOPE_H
#define JD_CPP_GYROSCOPE_H

#include "JdSensor.h"
#include "jacdac-c/jacdac/dist/c/gyroscope.h"

class JdGyroscope : public JdSensor {
public:
    explicit JdGyroscope();
    void process() override;
    void handle_packet(jd_packet_t *pkt) override;

    uint32_t getServiceClass() override {
        return JD_SERVICE_CLASS_GYROSCOPE;
    }
private:

    void *generic_sensor_get_reading() override;
    virtual const jd_gyroscope_rotation_rates_t * get_reading() = 0;
    jd_gyroscope_rotation_rates_t sample_ {};
    uint32_t nextSample_ = 0;

};


#endif //JD_CPP_GYROSCOPE_H