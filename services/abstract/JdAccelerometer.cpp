#include "JdAccelerometer.h"
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.

extern "C" {
#include "jacdac-c/services/jd_services.h"
#include "jacdac-c/services/interfaces/jd_sensor_api.h"
#include "jacdac-c/jacdac/dist/c/accelerometer.h"

}
#ifdef PIN_ACC_INT
extern "C" {
#include "lib.h"
}
#endif

// shake/gesture detection based on
// https://github.com/lancaster-university/codal-core/blob/master/source/driver-models/Accelerometer.cpp

/*
MakeCode accelerator position:
Laying flat: 0,0,-1000
Standing on left edge: -1000,0,0
Standing on bottom edge: 0,1000,0
*/

// values for QMA7981
#define SAMPLING_PERIOD (7695 * 2) // 64.98Hz

uint8_t gyroscope_pending;

#ifdef PIN_ACC_INT
static uint8_t got_accelerometer_int;
void JdAccelerometer::accelerometer_int(void) {
    got_accelerometer_int = 1;
}
#endif

#define JD_ACCELEROMETER_EV_NONE 0

#define ACCELEROMETER_REST_TOLERANCE 200
#define ACCELEROMETER_TILT_TOLERANCE 200
#define ACCELEROMETER_FREEFALL_TOLERANCE 400
#define ACCELEROMETER_SHAKE_TOLERANCE (400 << 10)
#define ACCELEROMETER_SHAKE_COUNT_THRESHOLD 4

#define ACCELEROMETER_GESTURE_DAMPING 5
#define ACCELEROMETER_SHAKE_DAMPING 10
#define ACCELEROMETER_SHAKE_RTX 30

#define ACCELEROMETER_REST_THRESHOLD (ACCELEROMETER_REST_TOLERANCE * ACCELEROMETER_REST_TOLERANCE)
#define ACCELEROMETER_FREEFALL_THRESHOLD                                                           \
    ((uint32_t)ACCELEROMETER_FREEFALL_TOLERANCE * (uint32_t)ACCELEROMETER_FREEFALL_TOLERANCE)





#define sample sample_
#define shake shake_

void JdAccelerometer::emit_g_event(int ev) {
    if (g_events_ & (1 << ev))
        return;
    g_events_ |= 1 << ev;
    jd_send_event( (srv_t*) state_, ev);
}

uint16_t JdAccelerometer::instantaneousPosture(uint32_t force) {
    bool shakeDetected = false;

    // Test for shake events.
    // We detect a shake by measuring zero crossings in each axis. In other words, if we see a
    // strong acceleration to the left followed by a strong acceleration to the right, then we can
    // infer a shake. Similarly, we can do this for each axis (left/right, up/down, in/out).
    //
    // If we see enough zero crossings in succession (ACCELEROMETER_SHAKE_COUNT_THRESHOLD), then we
    // decide that the device has been shaken.
    if ((sample.x < -ACCELEROMETER_SHAKE_TOLERANCE && shake.x) ||
        (sample.x > ACCELEROMETER_SHAKE_TOLERANCE && !shake.x)) {
        shakeDetected = true;
        shake.x = !shake.x;
    }

    if ((sample.y < -ACCELEROMETER_SHAKE_TOLERANCE && shake.y) ||
        (sample.y > ACCELEROMETER_SHAKE_TOLERANCE && !shake.y)) {
        shakeDetected = true;
        shake.y = !shake.y;
    }

    if ((sample.z < -ACCELEROMETER_SHAKE_TOLERANCE && shake.z) ||
        (sample.z > ACCELEROMETER_SHAKE_TOLERANCE && !shake.z)) {
        shakeDetected = true;
        shake.z = !shake.z;
    }

    // If we detected a zero crossing in this sample period, count this.
    if (shakeDetected && shake.count < ACCELEROMETER_SHAKE_COUNT_THRESHOLD) {
        shake.count++;

        if (shake.count == 1)
            shake.timer = 0;

        if (shake.count == ACCELEROMETER_SHAKE_COUNT_THRESHOLD) {
            shake.shaken = 1;
            shake.timer = 0;
            return JD_ACCELEROMETER_EV_SHAKE;
        }
    }

    // measure how long we have been detecting a SHAKE event.
    if (shake.count > 0) {
        shake.timer++;

        // If we've issued a SHAKE event already, and sufficient time has assed, allow another SHAKE
        // event to be issued.
        if (shake.shaken && shake.timer >= ACCELEROMETER_SHAKE_RTX) {
            shake.shaken = 0;
            shake.timer = 0;
            shake.count = 0;
        }

        // Decay our count of zero crossings over time. We don't want them to accumulate if the user
        // performs slow moving motions.
        else if (!shake.shaken && shake.timer >= ACCELEROMETER_SHAKE_DAMPING) {
            shake.timer = 0;
            if (shake.count > 0)
                shake.count--;
        }
    }

    if (force < ACCELEROMETER_FREEFALL_THRESHOLD)
        return JD_ACCELEROMETER_EV_FREEFALL;

    // Determine our posture.
    if ((sample.x >> 10) < (-1000 + ACCELEROMETER_TILT_TOLERANCE))
        return JD_ACCELEROMETER_EV_TILT_LEFT;

    if ((sample.x >> 10) > (1000 - ACCELEROMETER_TILT_TOLERANCE))
        return JD_ACCELEROMETER_EV_TILT_RIGHT;

    if ((sample.y >> 10) < (-1000 + ACCELEROMETER_TILT_TOLERANCE))
        return JD_ACCELEROMETER_EV_TILT_DOWN;

    if ((sample.y >> 10) > (1000 - ACCELEROMETER_TILT_TOLERANCE))
        return JD_ACCELEROMETER_EV_TILT_UP;

    if ((sample.z >> 10) < (-1000 + ACCELEROMETER_TILT_TOLERANCE))
        return JD_ACCELEROMETER_EV_FACE_UP;

    if ((sample.z >> 10) > (1000 - ACCELEROMETER_TILT_TOLERANCE))
        return JD_ACCELEROMETER_EV_FACE_DOWN;

    return JD_ACCELEROMETER_EV_NONE;
}

#define G(g) ((g * 1024) * (g * 1024))
void JdAccelerometer::process_events() {
    // works up to 16g
    uint32_t force = (sample.x >> 10) * (sample.x >> 10) + (sample.y >> 10) * (sample.y >> 10) +
                     (sample.z >> 10) * (sample.z >> 10);

    if (force > G(2)) {
        impulseSigma_ = 0;
        if (force > G(2))
            emit_g_event(JD_ACCELEROMETER_EV_FORCE_2G);
        if (force > G(3))
            emit_g_event(JD_ACCELEROMETER_EV_FORCE_3G);
        if (force > G(6))
            emit_g_event(JD_ACCELEROMETER_EV_FORCE_6G);
        if (force > G(8))
            emit_g_event(JD_ACCELEROMETER_EV_FORCE_8G);
    }

    if (impulseSigma_ < 5)
        impulseSigma_++;
    else
        g_events_ = 0;

    // Determine what it looks like we're doing based on the latest sample...
    uint16_t g = instantaneousPosture(force);

    if (g == JD_ACCELEROMETER_EV_SHAKE) {
        jd_send_event( (srv_t*) state_, JD_ACCELEROMETER_EV_SHAKE);
    } else {
        // Perform some low pass filtering to reduce jitter from any detected effects
        if (g == currentGesture_) {
            if (sigma_ < ACCELEROMETER_GESTURE_DAMPING)
                sigma_++;
        } else {
            currentGesture_ = g;
            sigma_ = 0;
        }

        // If we've reached threshold, update our record and raise the relevant event...
        if (currentGesture_ != lastGesture_ &&
            sigma_ >= ACCELEROMETER_GESTURE_DAMPING) {
            lastGesture_ = currentGesture_;
            if (lastGesture_ != JD_ACCELEROMETER_EV_NONE)
                jd_send_event( (srv_t*) state_, lastGesture_);
        }
    }
}

void JdAccelerometer::process() {

    if (!jd_should_sample(&nextSample_, SAMPLING_PERIOD))
        return;

    sensor_maybe_process();
    void *tmp = sensor_get_reading();
    if (tmp) {
        sample_ = *(jd_accelerometer_forces_t*) tmp;
    }

    process_events();

    sensor_process_simple(&sample, sizeof(sample));
}

void JdAccelerometer::handle_packet(jd_packet_t *pkt) {
    sensor_handle_packet_simple(pkt, &sample, sizeof(sample));
}

void *JdAccelerometer::generic_sensor_get_reading() {
    return (void *) accelerometer_get_reading();
}


JdAccelerometer::JdAccelerometer() = default;
