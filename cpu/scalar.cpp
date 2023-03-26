
#include <stdio.h>

#ifdef _WIN32
#define WIN_DDL_EX __declspec(dllexport)
#else
#define WIN_DDL_EX
#endif

extern "C" WIN_DDL_EX void vector_add(const size_t matSize, const float *matA, const float *matB, float *matR) {
    for (unsigned i = 0; i < matSize; ++i) {
        matR[i] = matA[i] + matB[i];
    }
}
