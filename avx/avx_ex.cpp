#include <cassert>
#include <immintrin.h> // AVX + SSE4 header
#include <chrono>
#include <vector>
#include <iostream>

#include "avx_ex.h"

#define MAX_RAND_NUM 4096u
#define AVX2_WIDTH_FLOAT (int)(256/(sizeof(float)*8))
#define MATRIX_SIZE 1*AVX2_WIDTH_FLOAT
#define MATRIX_SIZE_FULL MATRIX_SIZE*MATRIX_SIZE

void avx::vector_add(const float *matA, const float *matB, float *matR) {
    for (int i = 0; i < (int)(MATRIX_SIZE_FULL/8); i++) {
        __m256 a = _mm256_load_ps(&matA[i*8]);
        __m256 b = _mm256_load_ps(&matB[i*8]);
        __m256 r = _mm256_add_ps(a, b);
        _mm256_store_ps(&matR[i*8], r);
    }
}

void sse::vector_add(const float *matA, const float *matB, float *matR) {
    for (int i = 0; i < (int)(MATRIX_SIZE_FULL/4); i++) {
        __m128 a = _mm_load_ps(&matA[i*4]);
        __m128 b = _mm_load_ps(&matB[i*4]);
        __m128 r = _mm_add_ps(a, b);
        _mm_store_ps(&matR[i*4], r);
    }
}

void linear::vector_add(const float *matA, const float *matB, float *matR) {
    for (size_t i = 0; i < MATRIX_SIZE_FULL; i++) {
        matR[i] = matA[i] + matB[i];
    }
}

void avx::matrix_mul(const float *matA, const float *matB, float *matR) {
    for (int y = 0; y < MATRIX_SIZE; y++) {
        for (int x = 0; x < MATRIX_SIZE; x++) {

            alignas(32) float subMatB[8];
            for (int n = 0; n < 8; n++) {
                subMatB[n] = matB[n*MATRIX_SIZE+x];
            }

            __m256 a = _mm256_load_ps(&matA[y]);
            __m256 b = _mm256_load_ps(subMatB);
            __m256 r = _mm256_mul_ps(a, b);

            __m128 r0 = _mm256_extractf128_ps(r, 0);
            __m128 r1 = _mm256_extractf128_ps(r, 1);
            //__m128 r1 = _mm256_extractf32x4_ps (r, 1);

            _mm256_store_ps(&matR[y*MATRIX_SIZE+x], r);
        }
    }
}

void linear::matrix_mul(const float *matA, const float *matB, float *matR) {
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
}

void reset_result(float *matR) {
    for (int y = 0; y < MATRIX_SIZE; y++) {
        for (int x = 0; x < MATRIX_SIZE; x++) {
            matR[y*MATRIX_SIZE+x] = 0;
        }
    }
}

int main() {
    // init matrix
    alignas(32) float matA[MATRIX_SIZE_FULL];
    alignas(32) float matB[MATRIX_SIZE_FULL];
    alignas(32) float matR[MATRIX_SIZE_FULL];

    // fill matrices
    // TODO cache-friendly?
    for (int y = 0; y < MATRIX_SIZE; y++) {
        for (int x = 0; x < MATRIX_SIZE; x++) {
            matA[y*MATRIX_SIZE+x] = static_cast<float>(rand() % MAX_RAND_NUM);
            matB[y*MATRIX_SIZE+x] = static_cast<float>(rand() % MAX_RAND_NUM);
        }
    }

    // TEST

    //linear::matrix_mul(matA, matB, matR);
    linear::vector_add(matA, matB, matR);

    std::cout << "Scalar result: ";

    for (int i = 0; i < MATRIX_SIZE_FULL; i++) {
        std::cout << static_cast<int>(matR[i]) << " ";
    }
    std::cout << std::endl;

    reset_result(matR);
    sse::vector_add(matA, matB, matR);

    std::cout << "SSE result: ";
    for (int i = 0; i < MATRIX_SIZE_FULL; i++) {
        std::cout << static_cast<int>(matR[i]) << " ";
    }
    std::cout << std::endl;

    reset_result(matR);
    avx::vector_add(matA, matB, matR);

    std::cout << "AVX result: ";
    for (int i = 0; i < MATRIX_SIZE_FULL; i++) {
        std::cout << static_cast<int>(matR[i]) << " ";
    }
    std::cout << std::endl;


    // BENCHMARK
    std::cout << std::endl;

    auto s = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 8192; i++) {
        //linear::matrix_mul(matA, matB, matR);
        linear::vector_add(matA, matB, matR);
    }

    auto e = std::chrono::high_resolution_clock::now();
    auto t = std::chrono::duration_cast<std::chrono::microseconds>(e - s);
    std::cout << "Scalar execution time: " << t.count() << " µs" << std::endl;

    s = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 8192; i++) {
        sse::vector_add(matA, matB, matR);
    }

    e = std::chrono::high_resolution_clock::now();
    t = std::chrono::duration_cast<std::chrono::microseconds>(e - s);
    std::cout << "SSE execution time: " << t.count() << " µs" << std::endl;

    s = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 8192; i++) {
        //avx::matrix_mul(matA, matB, matR);
        avx::vector_add(matA, matB, matR);
    }

    e = std::chrono::high_resolution_clock::now();
    t = std::chrono::duration_cast<std::chrono::microseconds>(e - s);
    std::cout << "AVX execution time: " << t.count() << " µs" << std::endl;
}
