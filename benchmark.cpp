
#include "constants.hpp"
#include "cpu/cpu.hpp"
#include "opencl/opencl.hpp"

// init matrices
alignas(64) float matA[MATRIX_SIZE_FULL];
alignas(64) float matB[MATRIX_SIZE_FULL];
alignas(64) float matBT[MATRIX_SIZE_FULL];
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

int main() {
    init_matrices();

    ModuleCpu moduleCpu;
    ModuleOpencl moduleOpencl;

    moduleCpu.inspect();

    moduleCpu.printInfo();
    moduleOpencl.printInfo();

    //moduleCpu.benchmark(MATRIX_SIZE_FULL, TEST_ITERATIONS, matA, matB, matR);
    moduleOpencl.benchmark(MATRIX_SIZE_FULL, TEST_ITERATIONS, matA, matB, matR);
}
