//
// Created by Ward on 15/04/2023.
//

#ifndef JD_CPP_JDREGISTER_H
#define JD_CPP_JDREGISTER_H

#include <cstdint>

class JdGenericRegister{
public:
    explicit JdGenericRegister(int regCode):regCode_(regCode){};
    struct register_data{
        const void* data;
        std::size_t size;
    };
    virtual register_data get() = 0;
    virtual void set(register_data reg) = 0;
    int getRegCode() const{
        return regCode_;
    };

    virtual ~JdGenericRegister() = default;

private:
    int regCode_;
};
template <typename T>
class JdRegister : public JdGenericRegister{
public:
    JdRegister(int regCode, T& reg): JdGenericRegister(regCode), register_(reg){};

    register_data get() override;
    void set(register_data reg) override;

private:
    T& register_;
};

template<typename T>
JdGenericRegister::register_data JdRegister<T>::get() {
    return {&register_, sizeof(T)};
}

#include <cstring>
#include <Arduino.h>
template<typename T>
void JdRegister<T>::set(JdGenericRegister::register_data reg) {

    if (reg.size < sizeof(T)) { //payload is < size of register
        //copy payload and fill the rest
        memcpy(&register_, reg.data, reg.size);

        //copied mostly blindly from source/jd_services.c service_handle_register
        //no clue why the fill changes based on that check
        int fill = !std::is_signed<T>()                       ? 0
            : ((*(uint8_t*)reg.data + (reg.size - 1)) & 0x80) ? 0xff
                                                              : 0;
        memset(&register_ + reg.size, fill, sizeof(T) - reg.size);
        return;
    }

    //copy payload to register
    memcpy(&register_, reg.data, sizeof(T));
}
#endif //JD_CPP_JDREGISTER_H
