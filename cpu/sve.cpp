#include <arm_sve.h>

#ifdef _WIN32
#define WIN_DLL_EX __declspec(dllexport)
#else
#define WIN_DLL_EX
#endif

extern "C" WIN_DLL_EX void vector_add(const size_t matSize, const float *matA, const float *matB, float *matR) {
    for (int i = 0; i < (int)(matSize / svcntw()); ++i) {
        svfloat32_t a = svld1_f32(&matA[i * svcntw()]);
        svfloat32_t b = svld1_f32(&matB[i * svcntw()]);
        svfloat32_t r = svadd_f32(a, b);
        svst1_f32(&matR[i * svcntw()], r);
    }
}
