//
// Created by Ward on 16/03/2023.
//

#include "JacdacServiceManager.h"
#include "jacdac-c/services/jd_services.h"

void JacdacServiceManagerClass::service_init() {
    for (const auto &service: services_){
        service->initialize();
    }
    for (const auto &legacyService: legacyServices_){
        legacyService();
    }
}

JacdacServiceManagerClass::~JacdacServiceManagerClass() {
    for (const auto &service: services_){
        delete service;
    }
}

void app_init_services() {
    JacdacServiceManager.service_init();
}

void JacdacServiceManagerClass::addService(JacdacService* service) {
    services_.push_back(service);
}

void JacdacServiceManagerClass::addService(jdservicefn_t service) {
    legacyServices_.push_back(service);
}

JacdacServiceManagerClass JacdacServiceManager;