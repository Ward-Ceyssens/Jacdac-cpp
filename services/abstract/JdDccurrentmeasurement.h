//
// generated on 2023-05-16.
// regenerate=0
//

#ifndef JD_CPP_DCCURRENTMEASUREMENT_H
#define JD_CPP_DCCURRENTMEASUREMENT_H

#include "JdSensor.h"
#include "jacdac-c/jacdac/dist/c/dccurrentmeasurement.h"

class JdDccurrentmeasurement : public JdSensor {
public:
    explicit JdDccurrentmeasurement(const char *measurement_name);
    void process() override;
    void handle_packet(jd_packet_t *pkt) override;

    uint32_t getServiceClass() override {
        return JD_SERVICE_CLASS_DC_CURRENT_MEASUREMENT;
    }

private:
    void *generic_sensor_get_reading() override {
        return get_reading();
    }
    virtual double * get_reading() = 0;

    double measurement_ = 0;
    const char *measurement_name_ = nullptr;

    
};


#endif //JD_CPP_DCCURRENTMEASUREMENT_H