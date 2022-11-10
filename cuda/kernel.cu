
__global__ void vector_add(const unsigned *testIter, const float *matA, const float *matB, float *matR) {
    int gid = blockIdx.x * blockDim.x + threadIdx.x;
    for (int i = 0; i < *testIter; ++i) {
        matR[gid] = matA[gid] + matB[gid];
    }
}
