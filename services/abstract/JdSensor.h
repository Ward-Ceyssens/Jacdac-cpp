//
// Created by Ward on 05/04/2023.
//

#ifndef JD_CPP_JDSENSOR_H
#define JD_CPP_JDSENSOR_H

#include "services/JacdacService.h"
extern "C" {
#include "jacdac-c/services/jd_sensor.h"
#include "jacdac-c/services/interfaces/jd_pins.h"
}
class JdSensor : public JacdacService {

public:
    JdSensor();
private:
    std::vector<JdGenericRegister*> sensor_regs_;

protected:
    uint8_t streaming_samples_ = 0;

    bool got_query_ = false,
    jd_inited_ = false,
    got_reading_ = false,
    reading_pending_ = false;

    uint32_t streaming_interval_ = 0;

    const sensor_api_t *api_ = nullptr;
    uint32_t next_streaming_ = 0;

protected:
    // api functions

    virtual void sensor_process() {};
    virtual void sensor_init() {};

    /**
     * makes the sensor go to sleep
     * @return true if the sensor has gone to sleep and needs to be reinitialized
     */
    virtual bool sensor_sleep() {return false;};
    virtual int32_t sensor_get_range() {return 0;};
    virtual int32_t *sensor_set_range(int32_t range) {return nullptr;}; // returns the range after setting

private:
    //implement this to return a void* cast version of your specific get reading functions
    //is used in sensor_get_reading
    virtual void * generic_sensor_get_reading() {return nullptr;};
    const struct sensor_range *ranges_;

protected:
    //sensor functions

    uint32_t get_status_code();

    void sensor_send_status();

    int respond_ranges();

    int sensor_handle_packet(jd_packet_t *pkt);

    /**
     * wrapper for getting a reading that makes sure the sensor is initiated and updates jacdac etc.
     * @warning do not override, implement generic_sensor_get_reading instead
     * @return same void* as generic_sensor_get_reading returns
     */
    void *sensor_get_reading();

    static const sensor_range_t *sensor_lookup_range(const sensor_range_t *ranges, int32_t requested);

    bool sensor_maybe_init();

    void sensor_maybe_process();

    void sensor_process_simple(const void *sample, uint32_t sample_size);

    int sensor_handle_packet_simple(jd_packet_t *pkt, const void *sample, uint32_t sample_size);

    int sensor_should_stream();

    static bool sensor_should_send_threshold_event(uint32_t *block, uint32_t debounce_ms, bool cond_ok);

    //env functions

    void env_sensor_process();

    int env_sensor_handle_packet(jd_packet_t *pkt);

    void env_set_value(env_reading_t *env, int32_t value, const int32_t *error_table);

    int32_t env_extrapolate_error(int32_t value, const int32_t *error_table);

    int32_t env_absolute_humidity(int32_t temp, int32_t humidity);

};


#endif //JD_CPP_JDSENSOR_H
