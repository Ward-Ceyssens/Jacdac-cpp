//
// generated on 2023-05-16.
// regenerate=0
//

#ifndef JD_CPP_DCVOLTAGEMEASUREMENT_H
#define JD_CPP_DCVOLTAGEMEASUREMENT_H

#include "JdSensor.h"
#include "jacdac-c/jacdac/dist/c/dcvoltagemeasurement.h"

class JdDcvoltagemeasurement : public JdSensor {
public:
    explicit JdDcvoltagemeasurement(uint8_t measurement_type, const char *measurement_name);
    void process() override;
    void handle_packet(jd_packet_t *pkt) override;

    uint32_t getServiceClass() override {
        return JD_SERVICE_CLASS_DC_VOLTAGE_MEASUREMENT;
    }

    uint8_t getMeasurementType() const;

    void setMeasurementType(uint8_t measurementType);

private:
    void *generic_sensor_get_reading() override {
        return get_reading();
    }
    virtual double * get_reading() = 0;

    double measurement_ = 0;
    uint8_t measurement_type_ = 0;
    const char *measurement_name_ = nullptr;

    
};


#endif //JD_CPP_DCVOLTAGEMEASUREMENT_H