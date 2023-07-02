//
// generated on 2023-05-09.
// regenerate=false
//

#ifndef JD_CPP_ACCELEROMETER_H
#define JD_CPP_ACCELEROMETER_H

#include "JdSensor.h"
#include "jacdac-c/jacdac/dist/c/accelerometer.h"
#include "jacdac-c/services/interfaces/jd_sensor_api.h"

class JdAccelerometer : public JdSensor {
public:
    explicit JdAccelerometer();
    void process() override;
    void handle_packet(jd_packet_t *pkt) override;

    uint32_t getServiceClass() override {
        return JD_SERVICE_CLASS_ACCELEROMETER;
    }

protected:
    virtual const jd_accelerometer_forces_t * accelerometer_get_reading() = 0;

private:
    void *generic_sensor_get_reading() override;

    uint8_t sigma_ = 0;
    uint8_t impulseSigma_ = 0;
    uint16_t g_events_ = 0;
    uint16_t currentGesture_ = 0, lastGesture_ = 0;
    uint32_t nextSample_ = 0;
    jd_accelerometer_forces_t sample_{};

    struct ShakeHistory {
        uint8_t shaken : 1, x : 1, y : 1, z : 1;
        uint8_t count;
        uint8_t timer;
    };
    struct ShakeHistory shake_{};

    void accelerometer_int(void);
	void emit_g_event(int ev);
	uint16_t instantaneousPosture(uint32_t force);
	void process_events();
};


#endif //JD_CPP_ACCELEROMETER_H