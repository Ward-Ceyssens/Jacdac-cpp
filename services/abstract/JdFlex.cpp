#include "JdFlex.h"
#include "services/concrete/JdGPIOFlex.h"
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.

extern "C" {
#include "jacdac-c/services/jd_services.h"
}
 void JdFlex::update() {
    sample_ = get_reading();
}

 void JdFlex::maybe_init() {
    if (sensor_maybe_init())
        update();
}

 void JdFlex::process() {
    maybe_init();

    if (jd_should_sample(&nextSample_, 9000) && jd_inited_)
        update();

    sensor_process_simple(&sample_, sizeof(sample_));
}

 void JdFlex::handle_packet(jd_packet_t *pkt) {
    sensor_handle_packet_simple(pkt, &sample_, sizeof(sample_));
}



