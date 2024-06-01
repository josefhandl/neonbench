
#include <string>
#include <vector>
#include <iostream>

#include "neonbench-system.hpp"
#include "neonbench-module.hpp"
#include "device-enum.hpp"

#include "cpu/cpu.hpp"
#include "ram/ram.hpp"

#ifdef HAVE_OPENCL
#include "opencl/opencl.hpp"
#endif
#ifdef HAVE_CUDA
#include "cuda/wrapper.hpp"
#endif

void printUsage() {
    std::cout << "Usage:" << std::endl;
    std::cout << "By default program show infro and start benchmark for all modules." << std::endl;
    std::cout << "Using the following options you can enable only specified modules and disable benchmark." << std::endl;
    std::cout << "Option:" << std::endl;
    std::cout << "\t--help  \tprint this help" << std::endl;
    std::cout << "\t--cpu   \tenable CPU module" << std::endl;
    std::cout << "\t--ram   \tenable RAM module" << std::endl;
    std::cout << "\t--opencl\tenable OpenCL module for both CPUs and GPUs" << std::endl;
    std::cout << "\t--cuda  \tenable CUDA module" << std::endl;
    std::cout << "\t--gpu   \tenable OpenCL and CUDA modules" << std::endl;
    std::cout << "\t--info  \tshow only info using enabled modules, without benchmark" << std::endl;
}

int main(int argc, char *argv[]) {
    NeonbenchSystem moduleSystem;

    bool cpu    = false;
    bool ram    = false;
    bool opencl = false;
    bool cuda   = false;
    bool specifiedDevices = false;
    bool benchmark = true;

    if (argc != 1) {
        std::vector<std::string> options;
        for (int i = 1; i < argc; ++i) {
            options.emplace_back(argv[i]);
        }

        for (const auto & option : options) {
            if (option == "--help") {
                printUsage();
                return 0;
            } else if (option == "--cpu") {
                specifiedDevices = true;
                cpu = true;
            } else if (option == "--ram") {
                specifiedDevices = true;
                ram = true;
            } else if (option == "--opencl") {
                specifiedDevices = true;
                opencl = true;
            } else if (option == "--cuda") {
                specifiedDevices = true;
                cuda = true;
            } else if (option == "--gpu") {
                specifiedDevices = true;
                opencl = true;
                cuda = true;
            } else if (option == "--info") {
                benchmark = false;
            } else {
                std::cerr << "Invalid option: " << option << std::endl;
                printUsage();
                return 1;
            }
        }
    }

    if (!specifiedDevices) {
        cpu    = true;
        ram    = true;
        opencl = true;
        cuda   = true;
    }

    // Init modules
    //===============
    std::vector<std::unique_ptr<NeonbenchModule>> modules;

    if (cpu) {
        modules.emplace_back(std::make_unique<ModuleCpu>(moduleSystem));
    }

    if (ram) {
        modules.emplace_back(std::make_unique<ModuleRam>());
    }

    #ifdef HAVE_OPENCL
    if (opencl)
        modules.emplace_back(std::make_unique<ModuleOpencl>());
    #endif
    #ifdef HAVE_CUDA
    if (cuda)
        modules.emplace_back(std::make_unique<ModulesCuda>());
    #endif

    // Print info
    //=============
    moduleSystem.printInfo();
    for (const auto & module : modules) {
        module->inspect();
        module->printInfo();
    }

    // Start benchmark
    //==================
    if (!benchmark) {
        return 0;
    }

    for (const auto & module : modules) {
        NeonbenchDevice deviceType = module->getDeviceType();
        switch (deviceType) {
            case NeonbenchDevice::cpu:
                module->benchmark_prepare(16*32, 16384*128*8);
                break;
            case NeonbenchDevice::ram:
                module->benchmark_prepare(64*1024*1024, 4);
                break;
            case NeonbenchDevice::gpu:
                module->benchmark_prepare(16*1024, 16384*128*8);

            default:
                break;
        }

        module->benchmark();
    }

    return 0;
}
