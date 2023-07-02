//
// generated on 2023-05-16.
// regenerate=0
//

#ifndef JD_CPP_BAROMETER_H
#define JD_CPP_BAROMETER_H

#include "JdSensor.h"
#include "jacdac-c/jacdac/dist/c/airpressure.h"
#include "jacdac-c/services/interfaces/jd_sensor_api.h"

class JdBarometer : public JdSensor {
public:
    explicit JdBarometer();
    void process() override;
    void handle_packet(jd_packet_t *pkt) override;

    uint32_t getServiceClass() override {
        return JD_SERVICE_CLASS_AIR_PRESSURE;
    }
    void sensor_process() override = 0;
    virtual const env_reading_t * get_reading() = 0;
private:

protected:
    void *generic_sensor_get_reading() override;


};


#endif //JD_CPP_BAROMETER_H