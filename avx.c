
#include <immintrin.h> // AVX + SSE4 header

void vector_add(const size_t matSize, const float *matA, const float *matB, float *matR) {
    for (int i = 0; i < (int)(matSize/8); i++) {
        __m256 a = _mm256_load_ps(&matA[i*8]);
        __m256 b = _mm256_load_ps(&matB[i*8]);
        __m256 r = _mm256_add_ps(a, b);
        _mm256_store_ps(&matR[i*8], r);
    }

    int overlap_start = (int)(matSize/8)*8;
    int overlap_end = overlap_start + matSize%8;

    for (int i = overlap_start; i < overlap_end; i++) {
        matR[i] = matA[i] + matB[i];
    }
}
