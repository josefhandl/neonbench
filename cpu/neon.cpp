#include <arm_neon.h>
#include <stdio.h>

#ifdef _WIN32
#define WIN_DLL_EX __declspec(dllexport)
#else
#define WIN_DLL_EX
#endif

extern "C" WIN_DLL_EX void vector_add(const size_t matSize, const float *matA, const float *matB, float *matR) {
    for (int i = 0; i < (int)(matSize / 4); ++i) {
        float32x4_t a = vld1q_f32(&matA[i * 4]);
        float32x4_t b = vld1q_f32(&matB[i * 4]);
        float32x4_t r = vaddq_f32(a, b);
        vst1q_f32(&matR[i * 4], r);
    }
}

