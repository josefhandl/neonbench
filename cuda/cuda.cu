
#include <iostream>
#include <chrono>

#include <cuda_runtime.h>

#include "kernel.cu"

// https://github.com/NVIDIA/cuda-samples/blob/master/Common/helper_cuda.h

// Beginning of GPU Architecture definitions
inline int _ConvertSMVer2Cores(int major, int minor) {
  // Defines for GPU Architecture types (using the SM version to determine
  // the # of cores per SM
  typedef struct {
    int SM;  // 0xMm (hexidecimal notation), M = SM Major version,
    // and m = SM minor version
    int Cores;
  } sSMtoCores;

  sSMtoCores nGpuArchCoresPerSM[] = {
      {0x30, 192},
      {0x32, 192},
      {0x35, 192},
      {0x37, 192},
      {0x50, 128},
      {0x52, 128},
      {0x53, 128},
      {0x60,  64},
      {0x61, 128},
      {0x62, 128},
      {0x70,  64},
      {0x72,  64},
      {0x75,  64},
      {0x80,  64},
      {0x86, 128},
      {0x87, 128},
      {0x90, 128},
      {-1, -1}};

  int index = 0;

  while (nGpuArchCoresPerSM[index].SM != -1) {
    if (nGpuArchCoresPerSM[index].SM == ((major << 4) + minor)) {
      return nGpuArchCoresPerSM[index].Cores;
    }

    index++;
  }

  // If we don't find the values, we default use the previous one
  // to run properly
  printf(
      "MapSMtoCores for SM %d.%d is undefined."
      "  Default to use %d Cores/SM\n",
      major, minor, nGpuArchCoresPerSM[index - 1].Cores);
  return nGpuArchCoresPerSM[index - 1].Cores;
}

inline const char* _ConvertSMVer2ArchName(int major, int minor) {
  // Defines for GPU Architecture types (using the SM version to determine
  // the GPU Arch name)
  typedef struct {
    int SM;  // 0xMm (hexidecimal notation), M = SM Major version,
    // and m = SM minor version
    const char* name;
  } sSMtoArchName;

  sSMtoArchName nGpuArchNameSM[] = {
      {0x30, "Kepler"},
      {0x32, "Kepler"},
      {0x35, "Kepler"},
      {0x37, "Kepler"},
      {0x50, "Maxwell"},
      {0x52, "Maxwell"},
      {0x53, "Maxwell"},
      {0x60, "Pascal"},
      {0x61, "Pascal"},
      {0x62, "Pascal"},
      {0x70, "Volta"},
      {0x72, "Xavier"},
      {0x75, "Turing"},
      {0x80, "Ampere"},
      {0x86, "Ampere"},
      {0x87, "Ampere"},
      {0x90, "Hopper"},
      {-1, "Graphics Device"}};

  int index = 0;

  while (nGpuArchNameSM[index].SM != -1) {
    if (nGpuArchNameSM[index].SM == ((major << 4) + minor)) {
      return nGpuArchNameSM[index].name;
    }

    index++;
  }

  // If we don't find the values, we default use the previous one
  // to run properly
  printf(
      "MapSMtoArchName for SM %d.%d is undefined."
      "  Default to use %s\n",
      major, minor, nGpuArchNameSM[index - 1].name);
  return nGpuArchNameSM[index - 1].name;
}

extern "C" int64_t cu_make_cuda_benchmark(const int device, unsigned matSize, unsigned testIter, const float *matA, const float *matB, float *matR) {
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
    cudaMemcpy(d_testIter, &testIter, sizeof(unsigned), cudaMemcpyHostToDevice);
    cudaMemcpy(d_matA, matA, sizeof(float) * matSize, cudaMemcpyHostToDevice);
    cudaMemcpy(d_matB, matB, sizeof(float) * matSize, cudaMemcpyHostToDevice);

    // run benchmark
    int blockSize = 128;
    int gridSize = matSize / blockSize;
    vector_add <<<gridSize, blockSize>>>(d_testIter, d_matA, d_matB, d_matR);

    cudaMemcpy(matR, d_matR, sizeof(float) * matSize, cudaMemcpyDeviceToHost);

    auto e = std::chrono::high_resolution_clock::now();
    auto t = std::chrono::duration_cast<std::chrono::microseconds>(e - s);

    cudaDeviceReset();
    return t.count();
}

extern "C" void cu_printInfo(int &deviceCount) {
    cudaError_t error_id = cudaGetDeviceCount(&deviceCount);
    if (error_id != cudaSuccess)
    {
        printf("cudaGetDeviceCount returned %d\n-> %s\n", (int)error_id, cudaGetErrorString(error_id));
        //std::cout << "Result = FAIL" << std::endl;
        //exit(EXIT_FAILURE);
    }

    if (deviceCount == 0)
        printf("No CUDA device available.\n");
        //return;

    int driverVersion = 0, runtimeVersion = 0;

    for (int dev = 0; dev < deviceCount; ++dev)
    {
        cudaSetDevice(dev);
        cudaDeviceProp deviceProp;
        cudaGetDeviceProperties(&deviceProp, dev);

        printf("Device %d: \"%s\"\n", dev, deviceProp.name);

        // Console log
        cudaDriverGetVersion(&driverVersion);
        cudaRuntimeGetVersion(&runtimeVersion);
        printf("  CUDA Driver Version / Runtime Version       %d.%d / %d.%d\n", driverVersion/1000, (driverVersion%100)/10, runtimeVersion/1000, (runtimeVersion%100)/10);
        printf("  CUDA Capability Major/Minor version number: %d.%d\n", deviceProp.major, deviceProp.minor);

        //char msg[256];
        //SPRINTF(msg, "  Total amount of global memory:                 %.0f MBytes (%llu bytes)\n",
        //        (float)deviceProp.totalGlobalMem/1048576.0f, (unsigned long long) deviceProp.totalGlobalMem);
        //printf("%s", msg);
        printf("  Multiprocessors: %2d\n", deviceProp.multiProcessorCount);
        //printf("major %d, minor %d\n", deviceProp.major, deviceProp.);

        printf("  CUDA Cores/MP:   %d\n", _ConvertSMVer2Cores(deviceProp.major, deviceProp.minor));
        printf("  CUDA Cores:      %d\n", _ConvertSMVer2Cores(deviceProp.major, deviceProp.minor) * deviceProp.multiProcessorCount);

        //printf("  GPU Max Clock rate:                            %.0f MHz (%0.2f GHz)\n", deviceProp.clockRate * 1e-3f, deviceProp.clockRate * 1e-6f);
        
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
