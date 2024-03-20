
#include <iostream>
#include <vector>
#include <fstream>
#include <memory>

#include "../neonbench-module.hpp"
#include "../benchmarked-object-float.hpp"

#ifdef _WIN32
#elif __APPLE__
#elif __linux__
#endif

extern "C" int64_t cu_make_cuda_benchmark(const int device, unsigned matSize, unsigned testIter, const float *matA, const float *matB, float *matR);
extern "C" void cu_printInfo(int &deviceCount);

//https://github.com/zchee/cuda-sample/blob/master/1_Utilities/deviceQuery/deviceQuery.cpp

class ModuleCuda : public NeonbenchModule {

private:
    std::unique_ptr<BenchmarkedObjectFloat> bo;

    int deviceCount = 0;

    int64_t make_cuda_benchmark(const int device, unsigned matSize, unsigned testIter, const float *matA, const float *matB, float *matR) {
        return cu_make_cuda_benchmark(device, matSize, testIter, matA, matB, matR);
    }

public:
    NeonbenchDevice getDeviceType() override {
        return NeonbenchDevice::gpu;
    }

    void inspect() override {
    }

    void printInfo() override {
        std::cout << "CUDA info:" << std::endl;
        std::cout << "--------------------------------------" << std::endl;
        
        cu_printInfo(deviceCount);
        
        std::cout << std::endl;
    }

    void benchmark_prepare(unsigned size, unsigned iterations) override {
        bo = std::make_unique<BenchmarkedObjectFloat>(size, iterations);
    }

    void benchmark() override {
        std::cout << "CUDA benchmark:" << std::endl;
        std::cout << "--------------------------------------" << std::endl;

        std::string points;

        for (int device = 0; device < deviceCount; ++device) {
            std::cout << "Device " << device << ": ";

            int64_t time;
            for (int i = 0; i < 4; ++i) {
                time = make_cuda_benchmark(device, bo->vectorSize, bo->iterations, bo->vecA, bo->vecB, bo->vecR);
            }
            compute_points(bo->vectorSize, bo->iterations, time, &points);

            bool benchmark_ok = test_benchmark(*bo);
            std::cout << (benchmark_ok ? points : "Invalid result") << std::endl;
        }

        std::cout << std::endl;
    }
};
