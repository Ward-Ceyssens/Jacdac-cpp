//
// Created by Ward on 10/04/2023.
//

#ifndef JD_CPP_JDLEDSINGLE_H
#define JD_CPP_JDLEDSINGLE_H

#include "services/JacdacService.h"
#include "jacdac-c/jacdac/dist/c/control.h"

class JdLedSingle : public JacdacService{
public:
    JdLedSingle();

protected:
    void process() override;

    void handle_packet(jd_packet_t *pkt) override;

    uint32_t getServiceClass() override;

    struct color_t{
        uint8_t r;
        uint8_t g;
        uint8_t b;
    };
private:
    virtual void show() = 0;
    virtual void update() = 0;

    virtual const color_t* getCurrentColor() = 0;
    virtual void animate(const jd_control_set_status_light_t *anim) = 0;

public:

    uint8_t getVariant() const;

    void setVariant(uint8_t variant);

    uint16_t getWaveLength() const;

    void setWaveLength(uint16_t waveLength);

    uint16_t getLedCount() const;

    void setLedCount(uint16_t ledCount);

    uint16_t getMaxPower() const;

    void setMaxPower(uint16_t maxPower);

    uint16_t getLuminousIntensity() const;

    void setLuminousIntensity(uint16_t luminousIntensity);

private:



    uint32_t step_sample_ = 0;

    uint8_t variant_ = 0;
    uint16_t wave_length_ = 0;
    uint16_t led_count_ = 0;
    uint16_t max_power_ = 0;
    uint16_t luminous_intensity_ = 0;
};


#endif //JD_CPP_JDLEDSINGLE_H
