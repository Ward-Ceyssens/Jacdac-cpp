#include "JdDccurrentmeasurement.h"
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.

extern "C" {
#include "jacdac-c/services/jd_services.h"
#include "jacdac-c/jacdac/dist/c/dccurrentmeasurement.h"


}


 void JdDccurrentmeasurement::process() {
    void *tmp = sensor_get_reading();
    memcpy(&measurement_, tmp, sizeof(measurement_));
    sensor_process_simple(&measurement_, sizeof(measurement_));
}

 void JdDccurrentmeasurement::handle_packet(jd_packet_t *pkt) {
    if (service_handle_register(pkt) ||
        service_handle_string_register(pkt, JD_DC_CURRENT_MEASUREMENT_REG_MEASUREMENT_NAME,
                                       measurement_name_))
        return;
    sensor_handle_packet_simple(pkt, &measurement_, sizeof(measurement_));
}


 JdDccurrentmeasurement::JdDccurrentmeasurement(const char *measurement_name) {
	addRegister(JD_DC_CURRENT_MEASUREMENT_REG_MEASUREMENT, measurement_);
    
    streaming_interval_ = 500;
    measurement_name_ = measurement_name;
}
