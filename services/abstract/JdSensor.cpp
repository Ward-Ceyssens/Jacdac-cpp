//
// Created by Ward on 05/04/2023.
//

#include "JdSensor.h"
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.

#include "jacdac-c/services/jd_services.h"


// this really should return jd_system_status_code_t but that structure is also 32 bits and starts
// with the status code enum, so for simplicity we just use uint32_t
uint32_t JdSensor::get_status_code() {
    if (!jd_inited_)
        return JD_STATUS_CODES_SLEEPING;
    if (!got_reading_)
        return JD_STATUS_CODES_INITIALIZING;
    return JD_STATUS_CODES_READY;
}

void JdSensor::sensor_send_status() {
    uint32_t payload = get_status_code();
    // DMESG("status: s=%d st=%d", service_index_, payload);
    jd_send_event_ext(reinterpret_cast<srv_t *>(state_), JD_EV_STATUS_CODE_CHANGED, &payload, sizeof(payload));
}

int JdSensor::respond_ranges() {
    const sensor_range_t *r = ranges_;
    if (!r)
        return 0;
    while (r->range)
        r++;
    int len = r - ranges_;
    uint32_t buf[len];
    r = ranges_;
    for (int i = 0; i < len; ++i)
        buf[i] = r[i].range;
    jd_send(state_->service_index, JD_GET(JD_REG_SUPPORTED_RANGES), buf, len * sizeof(uint32_t));
    return -JD_REG_SUPPORTED_RANGES;
}

int JdSensor::sensor_handle_packet(jd_packet_t *pkt) {
    switch (pkt->service_command) {
        case JD_GET(JD_REG_INTENSITY):
            jd_respond_u8(pkt, jd_inited_ ? 1 : 0);
            return -JD_REG_INTENSITY;
        case JD_GET(JD_REG_STATUS_CODE):
            jd_respond_u32(pkt, get_status_code());
            return -JD_REG_STATUS_CODE;
        case JD_GET(JD_REG_READING_RANGE):
            if (sensor_get_range() == 0)
                return 0;
            jd_respond_u32(pkt, sensor_get_range());
            return -JD_REG_READING_RANGE;
        case JD_SET(JD_REG_READING_RANGE):
            sensor_set_range(*(uint32_t *)pkt->data);
            return JD_REG_READING_RANGE;
        case JD_GET(JD_REG_SUPPORTED_RANGES):
            return respond_ranges();
        case JD_SET(JD_REG_INTENSITY):
            if (pkt->data[0]) {
                // this will make it initialize soon
                got_query_ = 1;
            } else {
                got_query_ = 0;
                streaming_samples_ = 0;
                // if sensor supports sleep and was already initialized, put it to sleep
                if (jd_inited_ && sensor_sleep()) {
                    got_reading_ = 0;
                    jd_inited_ = 0;
                    sensor_send_status();
                }
            }
            return JD_REG_INTENSITY;
    }

    int r = service_handle_register(pkt, sensor_regs_);
    switch (r) {
        case JD_REG_STREAMING_SAMPLES:
            if (streaming_samples_) {
                if (streaming_interval_ == 0)
                    streaming_interval_ = 100;
                next_streaming_ = now;
                got_query_ = 1;
            }
            break;
        case JD_REG_STREAMING_INTERVAL:
            if (streaming_interval_ < 20)
                streaming_interval_ = 20;
            if (streaming_interval_ > 100000)
                streaming_interval_ = 100000;
            break;
    }
    return r;
}

void *JdSensor::sensor_get_reading() {
//    if (!api_)
//        JD_PANIC();
    if (!jd_inited_)
        return NULL;
    void *r = generic_sensor_get_reading();
    if (r && !got_reading_) {
        got_reading_ = 1;
        sensor_send_status();
    }
    return r;
}

const sensor_range_t *JdSensor::sensor_lookup_range(const sensor_range_t *ranges, int32_t requested) {
    while (ranges->range) {
        if (ranges->range >= requested)
            return ranges;
        ranges++;
    }
    return ranges - 1; // return maximum possible one
}

bool JdSensor::sensor_maybe_init() {
    if (got_query_ && !jd_inited_) {
        got_reading_ = 0;
        jd_inited_ = 1;
        sensor_send_status();
        sensor_init();
        generic_sensor_get_reading();
        got_reading_ = 1;
        return true;
    }
    return false;
}

void JdSensor::sensor_maybe_process() {
    sensor_maybe_init();
    if (jd_inited_)
        sensor_process();
}

void JdSensor::sensor_process_simple(const void *sample, uint32_t sample_size) {
    sensor_maybe_process();
    if (sensor_should_stream())
        jd_send(state_->service_index, JD_GET(JD_REG_READING), sample, sample_size);
}

int JdSensor::sensor_handle_packet_simple(jd_packet_t *pkt, const void *sample,
                                uint32_t sample_size) {
    int r = sensor_handle_packet(pkt);

    if (pkt->service_command == JD_GET(JD_REG_READING)) {
        got_query_ = 1;
        jd_send(pkt->service_index, pkt->service_command, sample, sample_size);
        r = -JD_REG_READING;
    }

    if (r == 0)
        jd_send_not_implemented(pkt);

    return r;
}

int JdSensor::sensor_should_stream() {
    if (!streaming_samples_)
        return false;
    if (jd_should_sample(&next_streaming_, streaming_interval_ * 1000)) {
        streaming_samples_--;
        return true;
    }
    return false;
}

bool JdSensor::sensor_should_send_threshold_event(uint32_t *block, uint32_t debounce_ms, bool cond_ok) {
    if (*block == 0 || in_past(*block)) {
        if (cond_ok) {
            if (*block == 0) {
                *block = now + (debounce_ms << 10);
                return 1;
            } else {
                *block = now;
            }
        } else {
            *block = 0;
        }
    }
    return 0;
}


// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.

#include "jacdac-c/services/jd_services.h"
#include "jacdac-c/services/interfaces/jd_sensor_api.h"
#include "jacdac-c/jacdac/dist/c/eco2.h"

void JdSensor::env_sensor_process() {
    sensor_maybe_process();
    if (sensor_should_stream()) {
        const auto *env = static_cast<const env_reading_t *>(sensor_get_reading());
        if (env) {
            jd_send(state_->service_index, JD_GET(JD_REG_READING), &env->value, sizeof(env->value));
            jd_send(state_->service_index, JD_GET(JD_REG_READING_ERROR), &env->error,
                    sizeof(env->error));
        }
    }
}

int JdSensor::env_sensor_handle_packet(jd_packet_t *pkt) {
    int off;
    int32_t tmp;

    int r = sensor_handle_packet(pkt);
    if (r)
        return r;

    switch (pkt->service_command) {
        case JD_GET(JD_REG_READING):
            off = 0;
            break;
        case JD_GET(JD_REG_READING_ERROR):
            off = 1;
            break;
        case JD_GET(JD_REG_MIN_READING):
            off = 2;
            break;
        case JD_GET(JD_REG_MAX_READING):
            off = 3;
            break;
#if 0
            // this has been removed
    case JD_GET(JD_E_CO2_REG_CONDITIONING_PERIOD):
        if (state->api->conditioning_period) {
            tmp = state->api->conditioning_period();
            goto send_it;
        } else {
            jd_send_not_implemented(pkt);
            return 0;
        }
#endif
        default:
            jd_send_not_implemented(pkt);
            return 0;
    }

    got_query_ = 1;

    const auto *env = static_cast<const env_reading_t *>(sensor_get_reading());
    if (env == NULL)
        return 0;
    tmp = (&env->value)[off];

    // send_it:
    jd_send(pkt->service_index, pkt->service_command, &tmp, 4);
    return -(pkt->service_command & 0xfff);
}

void JdSensor::env_set_value(env_reading_t *env, int32_t value, const int32_t *error_table) {
    env->value = value;
    env->error = env_extrapolate_error(value, error_table);
}

int32_t JdSensor::env_extrapolate_error(int32_t value, const int32_t *error_table) {
    if (value < error_table[0])
        return error_table[1];
    int32_t prev = error_table[0];
    int idx = 2;
    for (;;) {
        if (error_table[idx] == -1 && error_table[idx + 1] == -1)
            return error_table[idx - 1];
        int32_t curr = error_table[idx];
        if (value <= curr) {
            int32_t size = curr - prev;
            int32_t pos = value - prev;
            int32_t e0 = error_table[idx - 1];
            int32_t e1 = error_table[idx + 1];
            return (pos * (e1 - e0) / size) + e0;
        }
        prev = curr;
        idx += 2;
    }
}

#define ABSHUM(t, h) (int)(t * (1 << 10)), (int)(h * (1 << 10))
static const int32_t abs_hum[] = {
        ABSHUM(-25, 0.6), ABSHUM(-20, 0.9), ABSHUM(-15, 1.6), ABSHUM(-10, 2.3), ABSHUM(-5, 3.4),
        ABSHUM(0, 4.8),   ABSHUM(5, 6.8),   ABSHUM(10, 9.4),  ABSHUM(15, 12.8), ABSHUM(20, 17.3),
        ABSHUM(25, 23),   ABSHUM(30, 30.4), ABSHUM(35, 39.6), ABSHUM(40, 51.1), ABSHUM(45, 65.4),
        ABSHUM(50, 83),   ERR_END,
};

// result is i22.10 g/m3
int32_t JdSensor::env_absolute_humidity(int32_t temp, int32_t humidity) {
    int32_t maxval = env_extrapolate_error(temp, abs_hum);
    return (maxval * humidity / 100) >> 10;
}

JdSensor::JdSensor() {
    sensor_regs_.emplace_back(new JdRegister<uint8_t>(JD_REG_STREAMING_SAMPLES, streaming_samples_));
    sensor_regs_.emplace_back(new JdRegister<uint32_t>(JD_REG_STREAMING_INTERVAL, streaming_interval_));
}

