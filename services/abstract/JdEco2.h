//
// generated on 2023-05-17.
// regenerate=0
//

#ifndef JD_CPP_ECO2_H
#define JD_CPP_ECO2_H

#include "JdSensor.h"
#include "jacdac-c/jacdac/dist/c/eco2.h"

class JdEco2 : public JdSensor {
public:
    explicit JdEco2();
    void process() override;
    void handle_packet(jd_packet_t *pkt) override;

    uint32_t getServiceClass() override {
        return JD_SERVICE_CLASS_E_CO2;
    }
    

private:

    

    
};


#endif //JD_CPP_ECO2_H