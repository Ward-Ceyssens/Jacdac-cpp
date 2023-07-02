#include "JdEco2.h"
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.

extern "C" {
#include "jacdac-c/services/jd_services.h"



}
 void JdEco2::process() {
    env_sensor_process();
}

 void JdEco2::handle_packet(jd_packet_t *pkt) {
    env_sensor_handle_packet(pkt);
}



 JdEco2::JdEco2() {
    
    streaming_interval_ = 1000;
}
