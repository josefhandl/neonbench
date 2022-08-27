
void __kernel vector_add(__global const unsigned *testIter, global const float *matA, global const float *matB, global float *matR) {
    size_t gid = get_global_id(0);
    for (unsigned i = 0; i < *testIter; ++i) {
        matR[gid] = matA[gid] + matB[gid];
    }
}