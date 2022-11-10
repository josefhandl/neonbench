
#include <iostream>
#include <vector>
#include <fstream>

#ifdef _WIN32
#elif __APPLE__
#elif __linux__
#endif

extern "C" int64_t cu_make_cuda_benchmark(const int device, unsigned matSize, unsigned testIter, const float *matA, const float *matB, float *matR);
extern "C" void cu_printInfo();

//https://github.com/zchee/cuda-sample/blob/master/1_Utilities/deviceQuery/deviceQuery.cpp

class ModuleCuda {

private:
    int deviceCount = 0;

    int64_t make_cuda_benchmark(const int device, unsigned matSize, unsigned testIter, const float *matA, const float *matB, float *matR) {
        return cu_make_cuda_benchmark(device, matSize, testIter, matA, matB, matR);
    }

public:
    void printInfo() {
        cu_printInfo();
    }

    void benchmark(unsigned matSize, unsigned testIter, const float *matA, const float *matB, float *matR) {
        std::cout << "CUDA Benchmark:" << std::endl;
        std::cout << "--------------------------------------" << std::endl;

        std::string points;

        for (int device = 0; device < deviceCount; ++device) {
            //std::cout << device.getInfo<CL_DEVICE_NAME>() << ": ";

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
