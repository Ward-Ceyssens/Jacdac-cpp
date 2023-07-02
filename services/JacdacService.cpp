//
// Created by Ward on 17/03/2023.
//

#include "JacdacService.h"
extern "C" {
#include "jacdac-c/inc/interfaces/jd_tx.h"
#include "jacdac-c/inc/jd_control.h"

}
void JacdacService_process(srv_state *state){
    state->jacdacService->process();
}
void JacdacService_handle_packet(srv_state *state, jd_packet_t *pkt) {
    state->jacdacService->handle_packet(pkt);

}

srv_t *JacdacService_allocate_service(const srv_vt_t *vt){
    return jd_allocate_service(vt);
}

void JacdacService::initialize() {
    allocateService(state_);
}

int JacdacService::service_handle_register_final(jd_packet_t *pkt) {
    int r = service_handle_register(pkt);
    if (r == 0)
        jd_send_not_implemented(pkt);
    return r;
}


int JacdacService::service_handle_register(jd_packet_t *pkt, std::vector<JdGenericRegister*> registers) {
    uint16_t cmd = pkt->service_command;
    bool is_get = JD_IS_GET(cmd);
    bool is_set = JD_IS_SET(cmd);
    if (!is_get && !is_set)
        return 0;

    if (is_set && pkt->service_size == 0)
        return 0;

    int reg = JD_REG_CODE(cmd);

    if (reg >= 0xf00) // these are reserved
        return 0;

    if (is_set && (reg & 0xf00) == 0x100)
        return 0; // these are read-only

    JdGenericRegister* jdRegister = nullptr;
    for (const auto &item: registers){
        if (item->getRegCode() == reg){
            jdRegister = item;
            break;
        }
    }
    if (!jdRegister){
        return 0; //register code not in list of registers
    }
    if (is_get){
        auto dataHandle = jdRegister->get();
        jd_send(pkt->service_index, pkt->service_command, dataHandle.data, dataHandle.size);
        return -reg;
    } else {
        jdRegister->set({pkt->data, pkt->service_size});
        return reg;
    }
}

JacdacService::~JacdacService() {
    for (auto reg: registers_) {
        delete reg;
    }
}



template <class srv_struct>
void JacdacService::allocateService(srv_struct &stateptr){
    auto JacdacService_vt = new const srv_vt_t {
            .service_class = getServiceClass(),
            .state_size = sizeof(*stateptr),
            .process = JacdacService_process,
            .handle_pkt = JacdacService_handle_packet,
    };
    auto oldState = *stateptr;
    delete stateptr;
    stateptr = (srv_struct) JacdacService_allocate_service(JacdacService_vt);
    oldState.vt = stateptr->vt;
    oldState.service_index = stateptr->service_index;
    *stateptr = oldState;
    stateptr->jacdacService = this;
}