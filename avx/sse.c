
#include <iostream>
#include <immintrin.h> // AVX + SSE4 header

#include "computation.h"


extern "C" Computation* create_object()
{
    return new Computation;
}

extern "C" void destroy_object( Computation* object )
{
    delete object;
}

void Computation::vector_add(const size_t matSize, const float *matA, const float *matB, float *matR) {
    for (int i = 0; i < (int)(matSize/4); i++) {
        __m128 a = _mm_load_ps(&matA[i*4]);
        __m128 b = _mm_load_ps(&matB[i*4]);
        __m128 r = _mm_add_ps(a, b);
        _mm_store_ps(&matR[i*4], r);
    }

    int overlap_start = (int)(matSize/4)*4;
    int overlap_end = overlap_start + matSize%4;

    for (int i = overlap_start; i < overlap_end; i++) {
        matR[i] = matA[i] + matB[i];
    }
}
