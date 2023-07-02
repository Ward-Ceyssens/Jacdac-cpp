//
// Created by Ward on 10/04/2023.
//

#include "JdLedSingle.h"
#include "services/concrete/JdGPIOLedSingle.h"
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
#ifdef __cplusplus
extern "C" {
#endif
#include "jacdac-c/services/jd_services.h"
#include "jacdac-c/jacdac/dist/c/ledsingle.h"
#ifdef __cplusplus
}
#endif
#define FRAME_US 65536

#define LOG JD_LOG
// #define LOG JD_NOLOG

JdLedSingle::JdLedSingle() {
    addRegister(JD_REG_VARIANT, variant_);
    addRegister(JD_LED_SINGLE_REG_WAVE_LENGTH, wave_length_);
    addRegister(JD_LED_SINGLE_REG_LED_COUNT, led_count_);
    addRegister(JD_LED_SINGLE_REG_MAX_POWER, max_power_);
    addRegister(JD_LED_SINGLE_REG_LUMINOUS_INTENSITY, luminous_intensity_);
}


void JdLedSingle::process() {
    if (!jd_should_sample(&step_sample_, FRAME_US))
        return;

    update();
}

void JdLedSingle::handle_packet(jd_packet_t *pkt) {
    switch (pkt->service_command) {
        case JD_GET(JD_LED_SINGLE_REG_COLOR): {
            jd_send(state_->service_index, pkt->service_command, getCurrentColor(), sizeof(color_t));
            break;
        }

        case JD_LED_SINGLE_CMD_ANIMATE:
            if (pkt->service_size >= sizeof(jd_control_set_status_light_t))
                animate((jd_control_set_status_light_t *)pkt->data);
            break;

        default:
            service_handle_register_final(pkt);
            break;
    }
}

uint32_t JdLedSingle::getServiceClass() {
    return JD_SERVICE_CLASS_LED_SINGLE;
}

uint8_t JdLedSingle::getVariant() const {
    return variant_;
}

void JdLedSingle::setVariant(uint8_t variant) {
    variant_ = variant;
}

uint16_t JdLedSingle::getWaveLength() const {
    return wave_length_;
}

void JdLedSingle::setWaveLength(uint16_t waveLength) {
    wave_length_ = waveLength;
}

uint16_t JdLedSingle::getLedCount() const {
    return led_count_;
}

void JdLedSingle::setLedCount(uint16_t ledCount) {
    led_count_ = ledCount;
}

uint16_t JdLedSingle::getMaxPower() const {
    return max_power_;
}

void JdLedSingle::setMaxPower(uint16_t maxPower) {
    max_power_ = maxPower;
}

uint16_t JdLedSingle::getLuminousIntensity() const {
    return luminous_intensity_;
}

void JdLedSingle::setLuminousIntensity(uint16_t luminousIntensity) {
    luminous_intensity_ = luminousIntensity;
}



