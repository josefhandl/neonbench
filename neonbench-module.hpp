
#ifndef NEONBENCH_MODULE_HPP
#define NEONBENCH_MODULE_HPP

#include "device-enum.hpp"

class NeonbenchModule {
public:
    virtual NeonbenchDevice getDeviceType() = 0;
    virtual void inspect() = 0;
    virtual void printInfo() = 0;
    virtual void benchmark_prepare(unsigned size, unsigned iterations) = 0;
    virtual void benchmark() = 0;
    virtual ~NeonbenchModule() {};

};

#endif //NEONBENCH_MODULE_HPP
