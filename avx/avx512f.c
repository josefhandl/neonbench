
#include <immintrin.h> // AVX + SSE4 header

void vector_add(const size_t matSize, const float *matA, const float *matB, float *matR) {
    for (int i = 0; i < (int)(matSize/16); i++) {
        __m512 a = _mm512_load_ps(&matA[i*16]);
        __m512 b = _mm512_load_ps(&matB[i*16]);
        __m512 r = _mm512_add_ps(a, b);
        _mm512_store_ps(&matR[i*16], r);
    }

    int overlap_start = (int)(matSize/16)*16;
    int overlap_end = overlap_start + matSize%16;

    for (int i = overlap_start; i < overlap_end; i++) {
        matR[i] = matA[i] + matB[i];
    }
}
