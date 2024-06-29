#include "hip/hip_runtime.h"

__global__ void vector_add(const unsigned *testIter, const float* matA, const float* matB, float* matR, const unsigned int matSize) {
    int gid = blockIdx.x * blockDim.x + threadIdx.x;
    for (int i = 0; i < *testIter; ++i) {
        matR[gid] = matA[gid] + matB[gid];
    }
}

