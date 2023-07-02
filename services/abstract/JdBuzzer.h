//
// Created by Ward on 10/04/2023.
//

#ifndef JD_CPP_JDBUZZER_H
#define JD_CPP_JDBUZZER_H

#include "services/JacdacService.h"
class JdBuzzer : public JacdacService {
public:
    JdBuzzer();

    uint8_t getVolume() const;

    void setVolume(uint8_t volume);

private:
    virtual void set_pwr(bool on) = 0;
    virtual void play_tone(uint32_t period, uint32_t duty) = 0;
    void process() override;

    void handle_packet(jd_packet_t *pkt) override;

    uint32_t getServiceClass() override;

    uint8_t
    volume_ = 255;

    uint32_t end_tone_time_ = 0;
    uint16_t period_ = 0;

};


#endif //JD_CPP_JDBUZZER_H
