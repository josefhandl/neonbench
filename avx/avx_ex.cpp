#include <cassert>

#include "avx_ex.h"

#define AVX2_WIDTH_FLOAT (int)(256/(sizeof(float)*8))

void avx::vector_add(const float *in1, const float *in2, float *ret) {
    __m256 a = _mm256_load_ps(in1);
    __m256 b = _mm256_load_ps(in2);
    __m256 c = _mm256_add_ps(a, b);
    _mm256_store_ps(ret, c);
}

void linear::vector_add(const float *in1, const float *in2, float *ret) {
    for (size_t i = 0; i < 8; i++) {
        ret[i] = in1[i] + in2[i];
    }
}

int main() {
    alignas(32) float a[AVX2_WIDTH_FLOAT] = { 1, 2, 3, 4, 5, 6, 7, 8 };
    alignas(32) float b[AVX2_WIDTH_FLOAT] = { 8, 7, 6, 5, 4, 3, 2, 1 };
    alignas(32) float r[AVX2_WIDTH_FLOAT];

    linear::vector_add(a, b, r);

    std::cout << "Scalar result: ";
    for (int i = 0; i < AVX2_WIDTH_FLOAT; i++) {
        std::cout << r[i] << " ";
    }
    std::cout << std::endl;

    avx::vector_add(a, b, r);

    std::cout << "AVX result: ";
    for (int i = 0; i < 8; i++) {
        std::cout << r[i] << " ";
    }
    std::cout << std::endl;

    auto s = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 8192; i++) {
        linear::vector_add(a, b, r);
    }

    auto e = std::chrono::high_resolution_clock::now();
    auto t = std::chrono::duration_cast<std::chrono::microseconds>(e - s);
    std::cout << "Scalar execution time: " << t.count() << " uSeconds" << std::endl;

    s = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 8192; i++) {
        avx::vector_add(a, b, r);
    }

    e = std::chrono::high_resolution_clock::now();
    t = std::chrono::duration_cast<std::chrono::microseconds>(e - s);
    std::cout << "AVX execution time: " << t.count() << " uSeconds" << std::endl;
}