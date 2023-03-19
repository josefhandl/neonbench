
#include "constants.hpp"

#include "cpu/cpu.hpp"
#ifdef HAVE_OPENCL
#include "opencl/opencl.hpp"
#endif
#ifdef HAVE_CUDA
#include "cuda/wrapper.hpp"
#endif

// init matrices
alignas(64) float matA[VECTOR_SIZE];
alignas(64) float matB[VECTOR_SIZE];
alignas(64) float matR[VECTOR_SIZE];

void init_matrices() {
    // fill matrices
    // TODO cache-friendly?
    for (int i = 0; i < VECTOR_SIZE; ++i) {
        matA[i] = static_cast<float>(rand() % MAX_RAND_NUM);
        matB[i] = static_cast<float>(rand() % MAX_RAND_NUM);
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

    moduleCpu.benchmark(VECTOR_SIZE, TEST_ITERATIONS, matA, matB, matR);
    #ifdef HAVE_OPENCL
    moduleOpencl.benchmark(VECTOR_SIZE, TEST_ITERATIONS, matA, matB, matR);
    #endif
    #ifdef HAVE_CUDA
    moduleCuda.benchmark(VECTOR_SIZE, TEST_ITERATIONS, matA, matB, matR);
    #endif
}
