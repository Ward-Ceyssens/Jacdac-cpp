//
// Created by Ward on 17/03/2023.
//

#ifndef JACDAC_COMPATIBILITY_JACDACSERVICE_H
#define JACDAC_COMPATIBILITY_JACDACSERVICE_H

//these 3 are the includes from jd_service_framework
//included separately because otherwise the extern C propagates and causes issues
#include "jacdac-c/inc/jd_config.h"
#include "jacdac-c/inc/jd_service_classes.h"
#include "jacdac-c/inc/jd_physical.h"
extern "C" {
    #include "jacdac-c/inc/jd_service_framework.h"
}


#include <cstdint>
#include <vector>
#include "registers/JdRegister.h"

class JacdacService {
public:
    /**
     * initializes the service, will be automatically invoked by the service manager,
     * use this for initializing anything that requires jacdac to have begun already
     * @note
     * if you override this function you must either call the base version too or call allocateService() yourself \n\n
     * if you extended the service state you must override this function and call allocateService(state_)
     * @example @code
     * void initialize() override { allocateService(state_); }; @endcode
     */
    virtual void initialize();
    virtual void process() = 0;
    virtual void handle_packet(jd_packet_t *pkt) = 0;
    /**
     * returns what type of service this is
     * @return type of service, defined in jacdac-c/jacdac/dist/c/\<service\>.h
     */
    virtual uint32_t getServiceClass() = 0;

    virtual ~JacdacService();

    struct srv_state {
        SRV_COMMON
        JacdacService* jacdacService;
    };
protected:

    srv_state* state_ = new srv_state;


    /**
     * Interprets packet as a register read/write, based on REG_DEFINITION() passed as 'sdesc'.
     * It will either read from or write to 'state', depending on register.
     * Returns 0 if the packet was not handled.
     * Returns register code, if the packet was handled as register write of that code.
     * Returns -register code, if the packet was handled as register read of that code.
     */
    int service_handle_register(jd_packet_t *pkt) { return service_handle_register(pkt, registers_);};
    int service_handle_register(jd_packet_t *pkt, std::vector<JdGenericRegister*> registers);

    /**
     * Like `service_handle_register()` but calls `jd_send_not_implemented()` for unknown packets.
     */
    int service_handle_register_final(jd_packet_t *pkt);

    /**
     * register a register for this service, order does not matter unlike jacdac-c
     * @tparam T the register's type
     * @param regCode the register code, likely defined as JD_REG_REGISTERNAME
     * @param reg the register itself
     */
    template <typename T>
    void addRegister(int regCode, T& reg){
        registers_.emplace_back(new JdRegister<T>(regCode, reg));
    }

private:
    template <class srv_struct>
    /**
     * properly allocate the service so Jacdac knows it exists and what type it is
     * @tparam srv_struct a struct descended from @ref srv_state
     * @param stateptr reference to the pointer pointing to the struct, will be replaced with the newly allocated pointer but all data retained
     */
    void allocateService(srv_struct &stateptr);

    std::vector<JdGenericRegister*> registers_ = std::vector<JdGenericRegister*>();
};


extern "C"{
struct srv_state : JacdacService::srv_state{};

void JacdacService_process(srv_state *state);
void JacdacService_handle_packet(srv_state *state, jd_packet_t *pkt);

srv_t *JacdacService_allocate_service(const srv_vt_t *vt);
}




#endif //JACDAC_COMPATIBILITY_JACDACSERVICE_H