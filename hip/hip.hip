#include <iostream>
#include <chrono>
#include <hip/hip_runtime.h>

#include "kernel.hip"

extern "C" int64_t hip_make_hip_benchmark(const int device, unsigned matSize, unsigned testIter, const float *matA, const float *matB, float *matR) {
    hipSetDevice(device);

    auto s = std::chrono::high_resolution_clock::now();

    // Allocate and set device memory
    unsigned *d_testIter;
    float *d_matA;
    float *d_matB;
    float *d_matR;
    hipMalloc((void**)&d_testIter, sizeof(unsigned));
    hipMalloc((void**)&d_matA, sizeof(float) * matSize);
    hipMalloc((void**)&d_matB, sizeof(float) * matSize);
    hipMalloc((void**)&d_matR, sizeof(float) * matSize);

    // Copy data to the device
    hipMemcpy(d_testIter, &testIter, sizeof(unsigned), hipMemcpyHostToDevice);
    hipMemcpy(d_matA, matA, sizeof(float) * matSize, hipMemcpyHostToDevice);
    hipMemcpy(d_matB, matB, sizeof(float) * matSize, hipMemcpyHostToDevice);

    // Run benchmark
    int blockSize = 128;
    int gridSize = (matSize + blockSize - 1) / blockSize;
    hipLaunchKernelGGL(vector_add, dim3(gridSize), dim3(blockSize), 0, 0, d_testIter, d_matA, d_matB, d_matR, matSize);

    hipMemcpy(matR, d_matR, sizeof(float) * matSize, hipMemcpyDeviceToHost);

    auto e = std::chrono::high_resolution_clock::now();
    auto t = std::chrono::duration_cast<std::chrono::microseconds>(e - s);

    hipFree(d_testIter);
    hipFree(d_matA);
    hipFree(d_matB);
    hipFree(d_matR);

    return t.count();
}

extern "C" void hip_printInfo(int &deviceCount) {
    hipError_t error_id = hipGetDeviceCount(&deviceCount);
    if (error_id != hipSuccess) {
        printf("hipGetDeviceCount returned %d\n-> %s\n", (int)error_id, hipGetErrorString(error_id));
        return;
    }

    if (deviceCount == 0) {
        printf("No HIP device available.\n");
        return;
    }

    for (int dev = 0; dev < deviceCount; ++dev) {
        hipSetDevice(dev);
        hipDeviceProp_t deviceProp;
        hipGetDeviceProperties(&deviceProp, dev);

        printf("Device %d: \"%s\"\n", dev, deviceProp.name);
        printf("  Total global memory: %lu bytes\n", deviceProp.totalGlobalMem);
        printf("  Shared memory per block: %lu bytes\n", deviceProp.sharedMemPerBlock);
        printf("  Registers per block: %d\n", deviceProp.regsPerBlock);
        printf("  Warp size: %d\n", deviceProp.warpSize);
        printf("  Max threads per block: %d\n", deviceProp.maxThreadsPerBlock);
        printf("  Max threads dimensions: [ %d, %d, %d ]\n", deviceProp.maxThreadsDim[0], deviceProp.maxThreadsDim[1], deviceProp.maxThreadsDim[2]);
        printf("  Max grid size: [ %d, %d, %d ]\n", deviceProp.maxGridSize[0], deviceProp.maxGridSize[1], deviceProp.maxGridSize[2]);
        printf("\n");
    }
}

