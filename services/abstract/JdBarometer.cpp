#include "JdBarometer.h"
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.

extern "C" {
#include "jacdac-c/services/jd_services.h"
}
void JdBarometer::process() {
    env_sensor_process();
}

void JdBarometer::handle_packet(jd_packet_t *pkt) {
    env_sensor_handle_packet(pkt);
}



JdBarometer::JdBarometer() {
    
    streaming_interval_ = 1000;
}

void *JdBarometer::generic_sensor_get_reading() {
    return (void *) get_reading();
}
