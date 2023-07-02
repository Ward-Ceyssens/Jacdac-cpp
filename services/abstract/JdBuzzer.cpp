//
// Created by Ward on 10/04/2023.
//

#include "JdBuzzer.h"
#include "services/concrete/JdGPIOBuzzer.h"
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
#ifdef __cplusplus
extern "C" {
#endif
#include "jacdac-c/services/jd_services.h"

#include "jacdac-c/jacdac/dist/c/buzzer.h"
#ifdef __cplusplus
}
#endif



void JdBuzzer::process() {
    if (period_ && in_past(end_tone_time_))
        period_ = 0;

    if (period_ == 0) {
        set_pwr(0);
        return;
    }
}

void JdBuzzer::handle_packet(jd_packet_t *pkt) {
    switch (pkt->service_command) {
        case JD_BUZZER_CMD_PLAY_TONE:
            // ensure input is big enough
            if (pkt->service_size >= 6) {
                auto *d = static_cast<jd_buzzer_play_tone_t *>((void *) pkt->data);
                end_tone_time_ = now + d->duration * 1000;
                period_ = d->period;
                play_tone(period_, d->duty);
            }
            process();
            break;
        default:
            service_handle_register_final(pkt);
            break;
    }
}

uint32_t JdBuzzer::getServiceClass() {
    return JD_SERVICE_CLASS_BUZZER;
}

uint8_t JdBuzzer::getVolume() const {
    return volume_;
}

void JdBuzzer::setVolume(uint8_t volume) {
    volume_ = volume;
}

JdBuzzer::JdBuzzer() {
    addRegister(JD_REG_INTENSITY, volume_);

}
