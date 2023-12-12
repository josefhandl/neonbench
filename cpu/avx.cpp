
#include <immintrin.h>

#ifdef _WIN32
#define WIN_DLL_EX __declspec(dllexport)
#else
#define WIN_DLL_EX
#endif

extern "C" WIN_DLL_EX void vector_add(const size_t matSize, const float *matA, const float *matB, float *matR) {
    for (int i = 0; i < (int)(matSize/8); ++i) {
        __m256 a = _mm256_load_ps(&matA[i*8]);
        __m256 b = _mm256_load_ps(&matB[i*8]);
        __m256 r = _mm256_add_ps(a, b);
        _mm256_store_ps(&matR[i*8], r);
    }
}
