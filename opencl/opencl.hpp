
#include <iostream>
#include <vector>

#include <CL/opencl.hpp>

#define CL_HPP_ENABLE_EXCEPTIONS
#define CL_HPP_TARGET_OPENCL_VERSION 300

class ModuleOpencl {

private:

public:
    void printInfo() {
        std::cout << "OpenCL info:" << std::endl;
        std::cout << "--------------------------------------" << std::endl;

        std::vector<cl::Platform> platformList;
        cl::Platform::get(&platformList);

        if (platformList.size() == 0) {
            std::cout << "No OpenCL platform available" << std::endl;
            return;
        }

        int deviceTotal = 0;
        for (cl::Platform platform : platformList) {
            std::vector<cl::Device> deviceList;
            platform.getDevices(CL_DEVICE_TYPE_ALL, &deviceList);

            if (deviceList.size() == 0) {
                std::cout << "No OpenCL device available for this platform" << std::endl;
            }

            deviceTotal += deviceList.size();

            std::cout << "Platform name: " << platform.getInfo<CL_PLATFORM_NAME>() << std::endl;
            std::cout << "Devices: " << deviceList.size() << std::endl;

            for (cl::Device device : deviceList) {
                std::cout << "  Device Type: ";
                switch (device.getInfo<CL_DEVICE_TYPE>()) {
                case CL_DEVICE_TYPE_GPU:         std::cout << "GPU"; break;
                case CL_DEVICE_TYPE_CPU:         std::cout << "CPU"; break;
                case CL_DEVICE_TYPE_ACCELERATOR: std::cout << "Accelerator"; break;
                default:                         std::cout << "Unknown"; break;
                }
                std::cout << std::endl;

                std::cout << "  Name: " << device.getInfo<CL_DEVICE_NAME>() << std::endl;
                std::cout << "  Vendor: " << device.getInfo<CL_DEVICE_VENDOR>() << std::endl;
                std::cout << "  HW version: " << device.getInfo<CL_DEVICE_VERSION>() << std::endl;
                std::cout << "  OpenCL C version: " << device.getInfo<CL_DEVICE_OPENCL_C_VERSION>() << std::endl;
                std::cout << "  Driver version: " << device.getInfo<CL_DRIVER_VERSION>() << std::endl;
                std::cout << "  Compute Units: " << device.getInfo<CL_DEVICE_MAX_COMPUTE_UNITS>() << std::endl;

                std::cout << std::endl;
            }
        }

        std::cout << "------" << std::endl;
        std::cout << "Platforms: " << platformList.size() << std::endl;
        std::cout << "Total number of devices: " << deviceTotal << std::endl;

        std::cout << std::endl;
    }

};
