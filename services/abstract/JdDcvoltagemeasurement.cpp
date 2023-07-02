#include "JdDcvoltagemeasurement.h"
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.

extern "C" {
#include "jacdac-c/services/jd_services.h"
#include "jacdac-c/services/interfaces/jd_sensor_api.h"
#include "jacdac-c/jacdac/dist/c/dcvoltagemeasurement.h"


}


 void JdDcvoltagemeasurement::process() {
    void *tmp = sensor_get_reading();
    memcpy(&measurement_, tmp, sizeof(measurement_));
    sensor_process_simple(&measurement_, sizeof(measurement_));
}

 void JdDcvoltagemeasurement::handle_packet(jd_packet_t *pkt) {
    if (service_handle_register(pkt) ||
        service_handle_string_register(pkt, JD_DC_VOLTAGE_MEASUREMENT_REG_MEASUREMENT_NAME,
                                       measurement_name_))
        return;
    sensor_handle_packet_simple(pkt, &measurement_, sizeof(measurement_));
}


 JdDcvoltagemeasurement::JdDcvoltagemeasurement(uint8_t measurement_type, const char *measurement_name) {
	addRegister(JD_DC_VOLTAGE_MEASUREMENT_REG_MEASUREMENT, measurement_);
	addRegister(JD_DC_VOLTAGE_MEASUREMENT_REG_MEASUREMENT_TYPE, measurement_type_);
    
    streaming_interval_ = 500;
    measurement_type_ = measurement_type;
    measurement_name_ = measurement_name;
}

uint8_t JdDcvoltagemeasurement::getMeasurementType() const {
    return measurement_type_;
}

void JdDcvoltagemeasurement::setMeasurementType(uint8_t measurementType) {
    measurement_type_ = measurementType;
}
