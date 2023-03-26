
#include <immintrin.h>

#ifdef _WIN32
#define WIN_DDL_EX __declspec(dllexport)
#else
#define WIN_DDL_EX
#endif

extern "C" WIN_DDL_EX void vector_add(const size_t matSize, const float *matA, const float *matB, float *matR) {
    for (int i = 0; i < (int)(matSize/16); ++i) {
        __m512 a = _mm512_load_ps(&matA[i*16]);
        __m512 b = _mm512_load_ps(&matB[i*16]);
        __m512 r = _mm512_add_ps(a, b);
        _mm512_store_ps(&matR[i*16], r);
    }
}
