
#include <iostream>
#include <vector>
#include <fstream>

#include <cuda.h>

#include "kernel.cu"

#ifdef _WIN32
#elif __APPLE__
#elif __linux__
    #include <CL/opencl.hpp>
#endif


#define KERNEL_FILE "cuda/kernel.cu"
#define KERNEL_FUNCTION "vector_add"

//https://github.com/zchee/cuda-sample/blob/master/1_Utilities/deviceQuery/deviceQuery.cpp

class ModuleCuda {

private:
    int deviceCount = 0;

    int64_t make_cuda_benchmark(const int device, unsigned matSize, unsigned testIter, const float *matA, const float *matB, float *matR) {

        cudaSetDevice(device);

        auto s = std::chrono::high_resolution_clock::now();

        // allocate and set device memory
        unsigned *d_testIter;
        float *d_matA;
        float *d_matB;
        float *d_matR;
        cudaMalloc((void**)&d_testIter, sizeof(unsigned));
        cudaMalloc((void**)&d_matA, sizeof(float) * matSize);
        cudaMalloc((void**)&d_matB, sizeof(float) * matSize);
        cudaMalloc((void**)&d_matR, sizeof(float) * matSize);

        // copy data to the device
        cudaMemcpy(d_testIter, testIter, sizeof(unsigned), cudaMemcpyHostToDevice);
        cudaMemcpy(d_matA, matA, sizeof(float) * matSize, cudaMemcpyHostToDevice);
        cudaMemcpy(d_matB, matB, sizeof(float) * matSize, cudaMemcpyHostToDevice);
        cudaMemcpy(d_matR, matR, sizeof(float) * matSize, cudaMemcpyHostToDevice);
        
        // run benchmark
        int blockSize = 128;
        int gridSize = matSize / blockSize;
        kernel <<<gridSize, blockSize>>>(d_testIter, d_matA, d_matB, d_matR);

        cudaMemcpy(testIter, d_testIter, sizeof(unsigned), cudaMemcpyDeviceToHost);

        auto e = std::chrono::high_resolution_clock::now();
        auto t = std::chrono::duration_cast<std::chrono::microseconds>(e - s);

        cudaDeviceReset();
        return t.count();
    }

public:
    void printInfo() {
        std::cout << "CUDA info:" << std::endl;
        std::cout << "--------------------------------------" << std::endl;

        cudaError_t error_id = cudaGetDeviceCount(&deviceCount);
        if (error_id != cudaSuccess)
        {
            std::cout << "cudaGetDeviceCount returned %d\n-> %s\n", (int)error_id, cudaGetErrorString(error_id) << std::endl;
            std::cout << "Result = FAIL" << std::endl;
            exit(EXIT_FAILURE);
        }

        if (deviceCount == 0)
            std::cout << "No CUDA device available" << std::endl;
            return;

        int dev, driverVersion = 0, runtimeVersion = 0;

        for (dev = 0; dev < deviceCount; ++dev)
        {   
            cudaSetDevice(dev);
            cudaDeviceProp deviceProp;
            cudaGetDeviceProperties(&deviceProp, dev);

            std::cout << "Device %d: \"%s\"\n", dev, deviceProp.name << std::endl;

            cudaDriverGetVersion(&driverVersion);
            cudaRuntimeGetVersion(&runtimeVersion);
            std::cout << "  CUDA Driver Version / Runtime Version: " << driverVersion/1000 << "." << (driverVersion%100)/10 << " / "
                      << runtimeVersion/1000 << "." << (runtimeVersion%100)/10 << std::endl;
            std::cout << "  CUDA Capability Major/Minor version number: " << deviceProp.major << "." << deviceProp.minor << std::endl;
            std::cout << msg, "  Total amount of global memory: " << (float)deviceProp.totalGlobalMem/1048576.0f
                      << " MBytes (" << (unsigned long long) deviceProp.totalGlobalMem << " bytes)" << std::endl;
            std::cout << "  Multiprocessors: " << deviceProp.multiProcessorCount << std::endl;
            std::cout << "  CUDA Cores / MP: " << _ConvertSMVer2Cores(deviceProp.major, deviceProp.minor) << std::endl;
            std::cout << "  CUDA Cores: " << _ConvertSMVer2Cores(deviceProp.major, deviceProp.minor) * deviceProp.multiProcessorCount << std::endl;
        }

        std::cout << "------" << std::endl;
        std::cout << "Total number of devices: " << deviceTotal << std::endl;

        std::cout << std::endl;
    }

    void benchmark(unsigned matSize, unsigned testIter, const float *matA, const float *matB, float *matR) {
        std::cout << "CUDA Benchmark:" << std::endl;
        std::cout << "--------------------------------------" << std::endl;

        std::string points;

        for (const cl::Device &device : cl_devices) {
            std::cout << device.getInfo<CL_DEVICE_NAME>() << ": ";

            int64_t time;
            for (int i = 0; i < 4; ++i) {
                time = make_cuda_benchmark(device, matSize, testIter, matA, matB, matR);
            }
            compute_points(matSize, testIter, time, &points);

            bool benchmark_ok = test_benchmark(matSize, matA, matB, matR);
            std::cout << (benchmark_ok ? points : "Failed") << std::endl;
        }
    }
};
