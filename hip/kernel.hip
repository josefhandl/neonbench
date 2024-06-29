#include "hip/hip_runtime.h"

extern "C" __global__ void vector_add(const unsigned *testIter, const float* matA, const float* matB, float* matR, const unsigned int matSize) {
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx < matSize) {
        matR[idx] = matA[idx] + matB[idx];
    }
}

