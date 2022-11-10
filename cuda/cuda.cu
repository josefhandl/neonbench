
#include <stdio.h>

#include "kernel.cu"

extern "C" int64_t cu_make_cuda_benchmark(const int device, unsigned matSize, unsigned testIter, const float *matA, const float *matB, float *matR) {
    cudaSetDevice(device);

    //auto s = std::chrono::high_resolution_clock::now();

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
    cudaMemcpy(d_testIter, &testIter, sizeof(unsigned), cudaMemcpyHostToDevice);
    cudaMemcpy(d_matA, matA, sizeof(float) * matSize, cudaMemcpyHostToDevice);
    cudaMemcpy(d_matB, matB, sizeof(float) * matSize, cudaMemcpyHostToDevice);
    cudaMemcpy(d_matR, matR, sizeof(float) * matSize, cudaMemcpyHostToDevice);

    // run benchmark
    int blockSize = 128;
    int gridSize = matSize / blockSize;
    vector_add <<<gridSize, blockSize>>>(d_testIter, d_matA, d_matB, d_matR);

    cudaMemcpy(&testIter, d_testIter, sizeof(unsigned), cudaMemcpyDeviceToHost);

    //auto e = std::chrono::high_resolution_clock::now();
    //auto t = std::chrono::duration_cast<std::chrono::microseconds>(e - s);

    cudaDeviceReset();
    return 1; //return t.count();
}

extern "C" void cu_printInfo() {
    /*
    std::cout << "CUDA info:" << std::endl;
    std::cout << "--------------------------------------" << std::endl;
*/
    int deviceCount;
    cudaError_t error_id = cudaGetDeviceCount(&deviceCount);
    if (error_id != cudaSuccess)
    {
        printf("cudaGetDeviceCount returned %d\n-> %s\n", (int)error_id, cudaGetErrorString(error_id));
        //std::cout << "Result = FAIL" << std::endl;
        exit(EXIT_FAILURE);
    }

    if (deviceCount == 0)
        printf("No CUDA device available");
        return;

    int dev, driverVersion = 0, runtimeVersion = 0;

    for (dev = 0; dev < deviceCount; ++dev)
    {
        cudaSetDevice(dev);
        cudaDeviceProp deviceProp;
        cudaGetDeviceProperties(&deviceProp, dev);

        printf("Device %d: \"%s\"\n", dev, deviceProp.name);
/*
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
        */
    }

/*
    std::cout << "------" << std::endl;
    std::cout << "Total number of devices: " << deviceTotal << std::endl;

    std::cout << std::endl;
    */
}
