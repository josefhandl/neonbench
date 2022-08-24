
#include <iostream>
#include <vector>

#include <CL/opencl.hpp>

#define CL_HPP_ENABLE_EXCEPTIONS
#define CL_HPP_TARGET_OPENCL_VERSION 300

class ModuleOpencl {

private:
    std::vector<cl::Device> cl_devices;

    int64_t make_opencl_benchmark(cl::Device &device, unsigned matSize, unsigned testIter, const float *matA, const float *matB, float *matR) {
        // https://raw.githubusercontent.com/ULHPC/tutorials/devel/gpu/opencl/slides.pdf
        // https://programmerclick.com/article/47811146604/

        cl::Context context({device});
        cl::CommandQueue queue(context, device);

        std::string kernel_code =
            "void kernel vector_add(global const unsigned *testIter, global const float *matA, global const float *matB, global float *matR) {"
            "    for (unsigned i = 0; i < *testIter; ++i) {"
            "        matR[get_global_id(0)] = matA[get_global_id(0)] + matB[get_global_id(0)];"
            "    }"
            "}";

        cl::Program::Sources sources;
        sources.push_back({ kernel_code.c_str(),kernel_code.length() });

        cl::Program program(context, sources);
        if(program.build({device})!=CL_SUCCESS){
            std::cout<<" Error building: "<<program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device)<<"\n";
            exit(1);
        }

        cl::KernelFunctor<cl::Buffer, cl::Buffer, cl::Buffer, cl::Buffer> vector_add(cl::Kernel(program, "vector_add"));
        cl::NDRange global(matSize);

        auto s = std::chrono::high_resolution_clock::now();

        cl::Buffer cl_testIter(context, CL_MEM_READ_ONLY, sizeof(unsigned));
        cl::Buffer cl_matA(context, CL_MEM_READ_ONLY, sizeof(float) * matSize);
        cl::Buffer cl_matB(context, CL_MEM_READ_ONLY, sizeof(float) * matSize);
        cl::Buffer cl_matR(context, CL_MEM_WRITE_ONLY, sizeof(float) * matSize);

        queue.enqueueWriteBuffer(cl_testIter, CL_TRUE, 0, sizeof(unsigned), &testIter);
        queue.enqueueWriteBuffer(cl_matA, CL_TRUE, 0, sizeof(float) * matSize, matA);
        queue.enqueueWriteBuffer(cl_matB, CL_TRUE, 0, sizeof(float) * matSize, matB);

        vector_add(cl::EnqueueArgs(queue, global), cl_testIter, cl_matA, cl_matB, cl_matR).wait();

        queue.enqueueReadBuffer(cl_matR, CL_TRUE, 0, sizeof(float) * matSize, matR);

        auto e = std::chrono::high_resolution_clock::now();
        auto t = std::chrono::duration_cast<std::chrono::microseconds>(e - s);

        return t.count();
    }

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
                cl_devices.push_back(device);

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

    void benchmark(unsigned matSize, unsigned testIter, const float *matA, const float *matB, float *matR) {
        std::cout << "OpenCL Benchmark:" << std::endl;
        std::cout << "--------------------------------------" << std::endl;

        std::string points;

        for (const cl::Device &device : cl_devices) {
            std::cout << device.getInfo<CL_DEVICE_NAME>() << ": ";
            int64_t time = make_opencl_benchmark(cl_devices.front(), matSize, testIter, matA, matB, matR);
            compute_points(matSize, testIter, time, &points);
            std::cout << points << std::endl;
        }
    }
};
