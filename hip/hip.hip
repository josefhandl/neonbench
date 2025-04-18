#ifndef NEONBENCH_HIP_HIP
#define NEONBENCH_HIP_HIP


#include <iostream>
#include <chrono>
#include <hip/hip_runtime.h>

#include "../tools.hpp"

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

        std::cout << "Device " << dev << ": " << deviceProp.name << std::endl;
        std::cout << "  Device PCI Domain ID: " << deviceProp.pciDomainID << std::endl;
        std::cout << "  Device PCI Bus ID: " << deviceProp.pciBusID << std::endl;
        std::cout << "  Device PCI Device ID: " << deviceProp.pciDeviceID << std::endl;

        // Total global memory size
        int64_t memSize = deviceProp.totalGlobalMem;
        std::string memSizeStr;
        num_bin_prefix((int64_t)memSize, &memSizeStr);
        std::cout << "  Total global memory: " << memSizeStr << "B" << std::endl;

        // Shared memory per block size
        int64_t sharedMemPerBlock = deviceProp.sharedMemPerBlock;
        std::string sharedMemPerBlockStr;
        num_bin_prefix((int64_t)sharedMemPerBlock, &sharedMemPerBlockStr);
        std::cout << "  Shared memory per block: " << sharedMemPerBlockStr << "B" << std::endl;

        std::cout << "  Registers per block: " << deviceProp.regsPerBlock << std::endl;
        std::cout << "  Warp size: " << deviceProp.warpSize << std::endl;
        std::cout << "  Max threads per multiprocessor: " << deviceProp.maxThreadsPerMultiProcessor << std::endl;
        std::cout << "  Compute units: " << deviceProp.multiProcessorCount << std::endl;
        std::cout << "  Memory bus width: " << deviceProp.memoryBusWidth << std::endl;
        std::cout << "  Memory clock rate: " << deviceProp.memoryClockRate << " kHz" << std::endl;

        if (deviceProp.ECCEnabled) {
            std::cout << "  ECC enabled: Yes" << std::endl;
        } else {
            std::cout << "  ECC enabled: No" << std::endl;
        }

        int64_t l2CacheSize = deviceProp.l2CacheSize;
        std::string l2CacheSizeStr;
        num_bin_prefix((int64_t)l2CacheSize, &l2CacheSizeStr);
        std::cout << "  L2 cache size: " << l2CacheSizeStr << "B" << std::endl;

        std::cout << "  GCN architecture name: " << deviceProp.gcnArchName << std::endl;

        std::cout << "  Clock rate: " << deviceProp.clockRate << " kHz" << std::endl;

        if (deviceProp.managedMemory) {
            std::cout << "  Unified shared memory: Yes" << std::endl;
        } else {
            std::cout << "  Unified shared memory: No" << std::endl;
        }

        std::cout << "  Compute mode: " << deviceProp.computeMode << std::endl;
        if (deviceProp.canMapHostMemory) {
            std::cout << "  Can map host memory: Yes" << std::endl;
        } else {
            std::cout << "  Can map host memory: No" << std::endl;
        }

        if (deviceProp.pageableMemoryAccess) {
            std::cout << "  Pageable memory access: Yes" << std::endl;
        } else {
            std::cout << "  Pageable memory access: No" << std::endl;
        }

        if (deviceProp.integrated) {
            std::cout << "  Integrated: Yes" << std::endl;
        } else {
            std::cout << "  Integrated: No" << std::endl;
        }

        if (deviceProp.isLargeBar) {
            std::cout << "  Large bar: Yes" << std::endl;
        } else {
            std::cout << "  Large bar: No" << std::endl;
        }

        // asic revision
        std::cout << "  ASIC revision: " << deviceProp.asicRevision << std::endl;

        // atomics
        std::cout << "  Device has atomic memory capabilities: " << deviceProp.hostNativeAtomicSupported << std::endl;

        if (deviceProp.unifiedAddressing) {
            std::cout << "  Unified addressing: Yes" << std::endl;
        } else {
            std::cout << "  Unified addressing: No" << std::endl;
        }

        std::cout << "  Concurrent copy and kernel execution: " << deviceProp.deviceOverlap << std::endl;

        std::cout << "  Concurrent kernels: " << deviceProp.concurrentKernels << std::endl;
        std::cout << "  Concurrent managed access: " << deviceProp.concurrentManagedAccess << std::endl;
        std::cout << "  Device type: " << deviceProp.deviceOverlap << std::endl;

        std::cout << "  Max threads per block: " << deviceProp.maxThreadsPerBlock << std::endl;
        std::cout << "  Max threads dimensions: [ " << deviceProp.maxThreadsDim[0] << ", " << deviceProp.maxThreadsDim[1] << ", " << deviceProp.maxThreadsDim[2] << " ]" << std::endl;
        std::cout << "  Max grid size: [ " << deviceProp.maxGridSize[0] << ", " << deviceProp.maxGridSize[1] << ", " << deviceProp.maxGridSize[2] << " ]" << std::endl;
        std::cout << std::endl;
    }
}

#endif //NEONBENCH_HIP_HIP