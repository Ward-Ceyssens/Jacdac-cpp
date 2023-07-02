#include "JdColor.h"
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.

extern "C" {
#include "jacdac-c/services/jd_services.h"
#include "jacdac-c/services/interfaces/jd_sensor_api.h"
#include "jacdac-c/jacdac/dist/c/color.h"



}
void JdColor::update() {
    auto sample_raw = (color_t*) sensor_get_reading();
    if (!sample_raw)
        return;
    uint32_t maxv = sample_raw->r;
    maxv = max(sample_raw->r, sample_raw->g);
    maxv = max(maxv, sample_raw->b);

    int d = (maxv >> 15);
    // int d = 1 << 16;
    // TODO update this when color service is updated to use 32 bits
    sample_[0] = sample_raw->r / d;
    sample_[1] = sample_raw->g / d;
    sample_[2] = sample_raw->b / d;
}

void JdColor::process() {
    // we'll only update once inited, but make sure we update immediately after init
    if (!jd_inited_)
        nextSample_ = now;

    sensor_process_simple(&sample_, sizeof(sample_));

    if (jd_should_sample(&nextSample_, 100000) && jd_inited_)
        update();
}

void JdColor::handle_packet(jd_packet_t *pkt) {
    sensor_handle_packet_simple(pkt, &sample_, sizeof(sample_));
}



JdColor::JdColor() = default;

