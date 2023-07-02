//
// generated on 2023-05-17.
// regenerate=0
//

#ifndef JD_CPP_FLEX_H
#define JD_CPP_FLEX_H

#include "JdSensor.h"
#include "jacdac-c/jacdac/dist/c/flex.h"

class JdFlex : public JdSensor {
public:
    JdFlex() = default;
    void process() override;
    void handle_packet(jd_packet_t *pkt) override;

    uint32_t getServiceClass() override {
        return JD_SERVICE_CLASS_FLEX;
    }
    

private:
    virtual uint16_t get_reading() =0;

    uint16_t sample_ = 0;
    uint32_t nextSample_ = 0;

    void update();
	void maybe_init();
};


#endif //JD_CPP_FLEX_H