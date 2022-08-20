#include <cassert>
#include <immintrin.h> // AVX + SSE4 header
#include <chrono>
#include <vector>
#include <iostream>
#include <dlfcn.h> // runtime library loading

#ifdef _WIN32
//  Windows
#define cpuid(info, x)    __cpuidex(info, x, 0)
#else
//  GCC Intrinsics
#include <cpuid.h>
void cpuid(int info[4], int InfoType){
    __cpuid_count(InfoType, 0, info[0], info[1], info[2], info[3]);
}
#endif

bool HW_SSE;
bool HW_AVX;
bool HW_AVX2;
bool HW_AVX512F;

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


void vector_add_scalar(const float *matA, const float *matB, float *matR) {
    for (size_t i = 0; i < MATRIX_SIZE_FULL; i++) {
        matR[i] = matA[i] + matB[i];
    }
}

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

void check_cpu_inst_set() {
    //https://stackoverflow.com/questions/6121792/how-to-check-if-a-cpu-supports-the-sse3-instruction-set
    //https://github.com/Mysticial/FeatureDetector
    // check sse, avx and avx512f support
    int info[4];
    cpuid(info, 0);
    int nIds = info[0];

    cpuid(info, 0x80000000);
    //unsigned nExIds = info[0];

    if (nIds >= 0x00000001){
        cpuid(info,0x00000001);
        HW_SSE = (info[3] & ((int)1 << 25)) != 0;
        HW_AVX = (info[2] & ((int)1 << 28)) != 0;
    }

    if (nIds >= 0x00000007){
        cpuid(info,0x00000007);
        HW_AVX2    = (info[1] & ((int)1 <<  5)) != 0;
        HW_AVX512F = (info[1] & ((int)1 << 16)) != 0;
    }
}

int64_t make_benchmark(const char *libName) {
    // https://youtu.be/_kIa4D7kQ8I
    
    // Linux + MacOS
    // dlopen()
    // dlsym()
    // dlclose()

    // Windows
    // LoadLibrary() / LoadLibraryEX()
    // GetProcAddress()
    // FreeLibrary()

    // Linux
    // lib.so

    // MacOS
    // lib.dylib

    // Windows
    // lib.dll

    void *handle = dlopen(libName, RTLD_LAZY); // RTLD_NOW
    if (handle == NULL){
        std::cerr << dlerror() << std::endl;
        exit(-1);
    }

    void (*vector_add) (const size_t, const float*, const float*, float*);
    vector_add = (void (*)(const size_t, const float*, const float*, float*))dlsym(handle, "vector_add");

    if (vector_add == NULL){
        std::cerr << dlerror() << std::endl;
        exit(-2);
    }

    auto s = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < TEST_ITERATIONS; i++) {
        vector_add(MATRIX_SIZE_FULL, matA, matB, matR);
    }

    auto e = std::chrono::high_resolution_clock::now();
    auto t = std::chrono::duration_cast<std::chrono::microseconds>(e - s);

    dlclose(handle);

    return t.count();
}

int main() {

    init_matrices();

    check_cpu_inst_set();

    // Scalar
    //---------
    auto s = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < TEST_ITERATIONS; i++) {
        vector_add_scalar(matA, matB, matR);
    }
    auto e = std::chrono::high_resolution_clock::now();
    auto t = std::chrono::duration_cast<std::chrono::microseconds>(e - s);
    std::cout << "Scalar: " << t.count() << " µs" << std::endl;

    // SSE
    //---------
    std::cout << "SSE:    ";
    if (HW_SSE)
        std::cout << make_benchmark("./sse.so") << " µs";
    else
        std::cout << "Not supported";
    std::cout << std::endl;

    // AVX
    //---------
    std::cout << "AVX:    ";
    if (HW_AVX)
        std::cout << make_benchmark("./avx.so") << " µs";
    else
        std::cout << "Not supported";
    std::cout << std::endl;

    // AVX512
    //---------
    std::cout << "AVX512: ";
    if (HW_AVX512F)
        std::cout << make_benchmark("./avx512f.so") << " µs";
    else
        std::cout << "Not supported";
    std::cout << std::endl;
}
