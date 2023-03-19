
#include "constants.hpp"

#include "cpu/cpu.hpp"
#ifdef HAVE_OPENCL
#include "opencl/opencl.hpp"
#endif
#ifdef HAVE_CUDA
#include "cuda/wrapper.hpp"
#endif

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

    #ifdef HAVE_OPENCL
    ModuleOpencl moduleOpencl;
    #endif
    #ifdef HAVE_CUDA
    ModuleCuda moduleCuda;
    #endif

    moduleCpu.inspect();

    moduleCpu.printInfo();
    #ifdef HAVE_OPENCL
    moduleOpencl.printInfo();
    #endif
    #ifdef HAVE_CUDA
    moduleCuda.printInfo();
    #endif

    moduleCpu.benchmark(MATRIX_SIZE_FULL, TEST_ITERATIONS, matA, matB, matR);
    #ifdef HAVE_OPENCL
    moduleOpencl.benchmark(MATRIX_SIZE_FULL, TEST_ITERATIONS, matA, matB, matR);
    #endif
    #ifdef HAVE_CUDA
    moduleCuda.benchmark(MATRIX_SIZE_FULL, TEST_ITERATIONS, matA, matB, matR);
    #endif
}
