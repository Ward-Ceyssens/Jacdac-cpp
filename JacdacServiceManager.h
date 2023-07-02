//
// Created by Ward on 16/03/2023.
//

#ifndef JACDAC_COMPATIBILITY_JACDACSERVICEMANAGER_H
#define JACDAC_COMPATIBILITY_JACDACSERVICEMANAGER_H


#include <vector>
#include <functional>
#include "services/JacdacService.h"
/**
 * handles initializing Jacdac services
 */
class JacdacServiceManagerClass {
public:
    typedef  void (*jdservicefn_t)();

    /**
     * initializes all added services
     *  @attention  this is automatically called by Jacdac \n
     *  DO NOT USE MANUALLY
     */
    void service_init();
    /**
     * add a service to be initialized
     * @param service a Jacdac service to be added
     */
    void addService(JacdacService* service);
    /**
     * add a service to be initialized using Jacdac-c function style instead of as an object
     * @param service Jacdac-c service_init function
     * @note this overload is meant to allow you to continue using Jacdac-c style service_init functions if necessary, use a JacdacService when possible
     */
    void addService(jdservicefn_t service);
    /**
     * handles initializing Jacdac services
     */
    JacdacServiceManagerClass()= default;

    virtual ~JacdacServiceManagerClass();
private:
    std::vector<JacdacService*> services_;
    std::vector<jdservicefn_t> legacyServices_;
};

extern JacdacServiceManagerClass JacdacServiceManager;

extern "C" void app_init_services();
#endif //JACDAC_COMPATIBILITY_JACDACSERVICEMANAGER_H
