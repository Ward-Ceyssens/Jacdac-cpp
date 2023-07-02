#include "JdGyroscope.h"
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.

extern "C" {
#include "jacdac-c/services/jd_services.h"
#include "jacdac-c/jacdac/dist/c/gyroscope.h"
}

 void JdGyroscope::process() {

    if (!jd_should_sample(&nextSample_, 9500))
        return;

    sensor_process();
    void *tmp = sensor_get_reading();
    if (tmp) {
        sample_ = *(jd_gyroscope_rotation_rates_t*)tmp;
    }

    sensor_process_simple(&sample_, sizeof(sample_));
}

 void JdGyroscope::handle_packet(jd_packet_t *pkt) {
    sensor_handle_packet_simple(pkt, &sample_, sizeof(sample_));
}



 JdGyroscope::JdGyroscope() = default;

void *JdGyroscope::generic_sensor_get_reading() {
    return (void *) get_reading();
}
