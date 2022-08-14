#include <cassert>
#include <immintrin.h> // AVX + SSE4 header
#include <chrono>
#include <vector>
#include <iostream>

#include "avx_ex.h"

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


// https://stackoverflow.com/questions/23189488/horizontal-sum-of-32-bit-floats-in-256-bit-avx-vector
#define _mm256_full_hadd_ps(v0, v1) \
        _mm256_hadd_ps(_mm256_permute2f128_ps(v0, v1, 0x20), \
                       _mm256_permute2f128_ps(v0, v1, 0x31))

#define TEST_ITERATIONS 16384*8
#define MAX_RAND_NUM 16u
#define AVX_WIDTH_FLOAT (int)(256/(sizeof(float)*8))
#define AVX512_WIDTH_FLOAT AVX_WIDTH_FLOAT*2
#define MATRIX_SIZE (1*AVX512_WIDTH_FLOAT)
#define MATRIX_SIZE_FULL MATRIX_SIZE*MATRIX_SIZE

bool HW_SSE;
bool HW_AVX;
bool HW_AVX2;
bool HW_AVX512F;

// https://www.appsloveworld.com/cplus/100/359/horizontal-sum-of-32-bit-floats-in-256-bit-avx-vector
static inline float _mm256_reduce_add_ps(__m256 x) {
    /* ( x3+x7, x2+x6, x1+x5, x0+x4 ) */
    const __m128 x128 = _mm_add_ps(_mm256_extractf128_ps(x, 1), _mm256_castps256_ps128(x));
    /* ( -, -, x1+x3+x5+x7, x0+x2+x4+x6 ) */
    const __m128 x64 = _mm_add_ps(x128, _mm_movehl_ps(x128, x128));
    /* ( -, -, -, x0+x1+x2+x3+x4+x5+x6+x7 ) */
    const __m128 x32 = _mm_add_ss(x64, _mm_shuffle_ps(x64, x64, 0x55));
    /* Conversion to float is a no-op on x86-64 */
    return _mm_cvtss_f32(x32);
}

bool linear::vector_add(const float *matA, const float *matB, float *matR) {
    for (size_t i = 0; i < MATRIX_SIZE_FULL; i++) {
        matR[i] = matA[i] + matB[i];
    }

    return true;
}

bool sse::vector_add(const float *matA, const float *matB, float *matR) {
    if (!HW_SSE) {
        return false;
    }

    for (int i = 0; i < (int)(MATRIX_SIZE_FULL/4); i++) {
        __m128 a = _mm_load_ps(&matA[i*4]);
        __m128 b = _mm_load_ps(&matB[i*4]);
        __m128 r = _mm_add_ps(a, b);
        _mm_store_ps(&matR[i*4], r);
    }

    int overlap_start = (int)(MATRIX_SIZE_FULL/4)*4;
    int overlap_end = overlap_start + MATRIX_SIZE_FULL%4;

    for (int i = overlap_start; i < overlap_end; i++) {
        matR[i] = matA[i] + matB[i];
    }

    return true;
}

bool avx::vector_add(const float *matA, const float *matB, float *matR) {
    if (!HW_AVX) {
        return false;
    }

    for (int i = 0; i < (int)(MATRIX_SIZE_FULL/8); i++) {
        __m256 a = _mm256_load_ps(&matA[i*8]);
        __m256 b = _mm256_load_ps(&matB[i*8]);
        __m256 r = _mm256_add_ps(a, b);
        _mm256_store_ps(&matR[i*8], r);
    }

    int overlap_start = (int)(MATRIX_SIZE_FULL/8)*8;
    int overlap_end = overlap_start + MATRIX_SIZE_FULL%8;

    for (int i = overlap_start; i < overlap_end; i++) {
        matR[i] = matA[i] + matB[i];
    }

    return true;
}

/*
bool avx512::vector_add(const float *matA, const float *matB, float *matR) {
    if (HW_AVX512F) {
        return false;
    }

    for (int i = 0; i < (int)(MATRIX_SIZE_FULL/16); i++) {
        __m512 a = _mm512_load_ps(&matA[i*16]);
        __m512 b = _mm512_load_ps(&matB[i*16]);
        __m512 r = _mm512_add_ps(a, b);
        _mm512_store_ps(&matR[i*16], r);
    }

    int overlap_start = (int)(MATRIX_SIZE_FULL/16)*16;
    int overlap_end = overlap_start + MATRIX_SIZE_FULL%16;

    for (int i = overlap_start; i < overlap_end; i++) {
        matR[i] = matA[i] + matB[i];
    }

    return true;
}
*/

bool linear::matrix_mul(const float *matA, const float *matB, float *matR) {
    // TODO two versions - cpu cache-unfriendly + friendly
    for (int y = 0; y < MATRIX_SIZE; y++) {
        for (int x = 0; x < MATRIX_SIZE; x++) {
            float num = 0;

            for (int n = 0; n < MATRIX_SIZE; n++) {
                num += matA[y*MATRIX_SIZE+n] * matB[n*MATRIX_SIZE+x];
            }

            matR[y*MATRIX_SIZE+x] = num;
        }
    }

    return true;
}

bool avx::matrix_mul(const float *matA, const float *matB, float *matR) {
    if (!HW_AVX) {
        return false;
    }

    for (int y = 0; y < MATRIX_SIZE; y++) {
        for (int x = 0; x < MATRIX_SIZE; x++) {
            __m256 a = _mm256_load_ps(&matA[y*MATRIX_SIZE]);
            __m256 b = _mm256_load_ps(&matB[x*MATRIX_SIZE]);
            __m256 r = _mm256_mul_ps(a, b);

            matR[y*MATRIX_SIZE+x] = _mm256_reduce_add_ps(r);
        }
    }

    return true;
}

void reset_result(float *matR) {
    for (int y = 0; y < MATRIX_SIZE; y++) {
        for (int x = 0; x < MATRIX_SIZE; x++) {
            matR[y*MATRIX_SIZE+x] = 0;
        }
    }
}

int main() {
    //https://stackoverflow.com/questions/6121792/how-to-check-if-a-cpu-supports-the-sse3-instruction-set
    //https://github.com/Mysticial/FeatureDetector
    // check sse, avx and avx512f support
    int info[4];
    cpuid(info, 0);
    int nIds = info[0];

    cpuid(info, 0x80000000);
    unsigned nExIds = info[0];

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

    // init matrix
    alignas(64) float matA[MATRIX_SIZE_FULL];
    alignas(64) float matB[MATRIX_SIZE_FULL];
    alignas(64) float matBT[MATRIX_SIZE_FULL];
    alignas(64) float matRf[MATRIX_SIZE_FULL];
    alignas(64) float matR[MATRIX_SIZE_FULL];

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

    // TEST
    linear::vector_add(matA, matB, matRf);

/*
    for (int i = 0; i < MATRIX_SIZE_FULL; i++) {
        std::cout << matRf[i] << " ";
    }
    std::cout << std::endl;
    */


    sse::vector_add(matA, matB, matR);
    bool ok_sse = true;
    for (int i = 0; ok_sse && i < MATRIX_SIZE_FULL; i++) {
        if (abs(matR[i] - matRf[i]) > 0.001) {
            ok_sse = false;
            std::cout << "SSE: wrong result" << std::endl << std::endl;
        }
    }

/*
    for (int i = 0; i < MATRIX_SIZE_FULL; i++) {
        std::cout << matR[i] << " ";
    }
    std::cout << std::endl;
    */

    reset_result(matR);
    // TODO transposed matrix for multiplication
    avx::vector_add(matA, matB, matR);
    bool ok_avx = true;
    for (int i = 0; ok_avx && i < MATRIX_SIZE_FULL; i++) {
        if (abs(matR[i] - matRf[i]) > 0.001) {
            ok_avx = false;
            std::cout << "AVX: wrong result" << std::endl << std::endl;
        }
    }

    reset_result(matR);
    //avx512::vector_add(matA, matB, matR);
    bool ok_avx512 = true;
    for (int i = 0; ok_avx512 && i < MATRIX_SIZE_FULL; i++) {
        if (abs(matR[i] - matRf[i]) > 0.001) {
            ok_avx512 = false;
            std::cout << "AVX512: wrong result" << std::endl << std::endl;
        }
    }

    // BENCHMARK
    std::cout << std::endl;

    auto s = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < TEST_ITERATIONS; i++) {
        linear::vector_add(matA, matB, matR);
    }

    auto e = std::chrono::high_resolution_clock::now();
    auto t = std::chrono::duration_cast<std::chrono::microseconds>(e - s);
    std::cout << "Scalar execution time: \t" << t.count() << " µs" << std::endl;

    s = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < TEST_ITERATIONS; i++) {
        sse::vector_add(matA, matB, matR);
    }

    e = std::chrono::high_resolution_clock::now();
    t = std::chrono::duration_cast<std::chrono::microseconds>(e - s);
    std::cout << "SSE execution time: \t" << t.count() << " µs" << std::endl;

    s = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < TEST_ITERATIONS; i++) {
        avx::vector_add(matA, matB, matR);
    }

    e = std::chrono::high_resolution_clock::now();
    t = std::chrono::duration_cast<std::chrono::microseconds>(e - s);
    std::cout << "AVX execution time: \t" << t.count() << " µs" << std::endl;
    
    s = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < TEST_ITERATIONS; i++) {
        //avx512::vector_add(matA, matB, matR);
    }

    e = std::chrono::high_resolution_clock::now();
    t = std::chrono::duration_cast<std::chrono::microseconds>(e - s);
    std::cout << "AVX512 execution time: \t" << t.count() << " µs" << std::endl;
}
