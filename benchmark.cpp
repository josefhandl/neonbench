
#include "cpu/cpu.hpp"
#include "ram/ram.hpp"
#ifdef HAVE_OPENCL
#include "opencl/opencl.hpp"
#endif
#ifdef HAVE_CUDA
#include "cuda/wrapper.hpp"
#endif

int main() {
    ModuleCpu moduleCpu;
    ModuleRam moduleRam;

    #ifdef HAVE_OPENCL
    ModuleOpencl moduleOpencl;
    #endif
    #ifdef HAVE_CUDA
    ModuleCuda moduleCuda;
    #endif

    moduleCpu.inspect();
    moduleRam.inspect();

    moduleCpu.printInfo();
    moduleRam.printInfo();
    #ifdef HAVE_OPENCL
    moduleOpencl.printInfo();
    #endif
    #ifdef HAVE_CUDA
    moduleCuda.printInfo();
    #endif

    moduleCpu.benchmark_prepare(16*32, 16384*128*8);
    moduleCpu.benchmark();
    moduleRam.benchmark_prepare(67108864, 4);
    moduleRam.benchmark();
    #ifdef HAVE_OPENCL
    moduleOpencl.benchmark_prepare(16*4096, 16384*128*32);
    moduleOpencl.benchmark();
    #endif
    #ifdef HAVE_CUDA
    moduleCuda.benchmark_prepare(16*4096, 16384*128*32);
    moduleCuda.benchmark();
    #endif
}
