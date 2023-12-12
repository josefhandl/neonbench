
#include <immintrin.h>

#ifdef _WIN32
#define WIN_DLL_EX __declspec(dllexport)
#else
#define WIN_DLL_EX
#endif

extern "C" WIN_DLL_EX void vector_add(const size_t matSize, const float *matA, const float *matB, float *matR) {
    for (int i = 0; i < (int)(matSize/4); ++i) {
        __m128 a = _mm_load_ps(&matA[i*4]);
        __m128 b = _mm_load_ps(&matB[i*4]);
        __m128 r = _mm_add_ps(a, b);
        _mm_store_ps(&matR[i*4], r);
    }
}
