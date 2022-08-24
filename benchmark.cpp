

#include "cpu/cpu.hpp"

#define TEST_ITERATIONS 16384*8
#define MAX_RAND_NUM 16u
#define AVX_WIDTH_FLOAT (int)(256/(sizeof(float)*8))
#define AVX512_WIDTH_FLOAT AVX_WIDTH_FLOAT*2
#define MATRIX_SIZE (1*AVX512_WIDTH_FLOAT)
#define MATRIX_SIZE_FULL MATRIX_SIZE*MATRIX_SIZE

// init matrices
alignas(64) float matA[MATRIX_SIZE_FULL];
alignas(64) float matB[MATRIX_SIZE_FULL];
alignas(64) float matBT[MATRIX_SIZE_FULL];
alignas(64) float matRf[MATRIX_SIZE_FULL];
alignas(64) float matR[MATRIX_SIZE_FULL];


void init_matrices() {
    // fill matrices
    // TODO cache-friendly?
    for (int y = 0; y < MATRIX_SIZE; y++) {
        for (int x = 0; x < MATRIX_SIZE; x++) {
            matA[y*MATRIX_SIZE+x] = static_cast<float>(rand() % MAX_RAND_NUM);

            float b = static_cast<float>(rand() % MAX_RAND_NUM);
            matB[y*MATRIX_SIZE+x] = b;
            matBT[x*MATRIX_SIZE+y] = b;
        }
    }
}

void reset_result_matrix(float *matR) {
    for (int y = 0; y < MATRIX_SIZE; y++) {
        for (int x = 0; x < MATRIX_SIZE; x++) {
            matR[y*MATRIX_SIZE+x] = 0;
        }
    }
}

int main() {
    init_matrices();

    ModuleCpu moduleCpu;
    moduleCpu.inspect();
    moduleCpu.printInfo();
    moduleCpu.benchmark(MATRIX_SIZE_FULL, TEST_ITERATIONS, matA, matB, matR);
}
