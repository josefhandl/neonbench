

    // Get OS info
    #ifdef _WIN32
    #else
        #include <sys/utsname.h>
    #endif


    std::cout << "System Info:" << std::endl;
    std::cout << "------------------------" << std::endl;

    // https://stackoverflow.com/questions/6315666/c-get-linux-distribution-name-version
    // TODO - posix only
    #ifdef _WIN32
    #else
        struct utsname osInfo;
        uname(&osInfo);
        std::cout << "OS: "
                  << osInfo.sysname << " "
                  << osInfo.nodename << " "
                  << osInfo.release << " "
                  << osInfo.version << " "
                  << osInfo.machine << std::endl;
    #endif






// https://stackoverflow.com/questions/23189488/horizontal-sum-of-32-bit-floats-in-256-bit-avx-vector
#define _mm256_full_hadd_ps(v0, v1) \
        _mm256_hadd_ps(_mm256_permute2f128_ps(v0, v1, 0x20), \
                       _mm256_permute2f128_ps(v0, v1, 0x31))

// https://www.appsloveworld.com/cplus/100/359/horizontal-sum-of-32-bit-floats-in-256-bit-avx-vector
static inline float _mm256_reduce_add_ps(__m256 x) {
    /* ( x3+x7, x2+x6, x1+x5, x0+x4 ) */
    const __m128 x128 = _mm_add_ps(_mm256_extractf128_ps(x, 1), _mm256_castps256_ps128(x));
    /* ( -, -, x1+x3+x5+x7, x0+x2+x4+x6 ) */
    const __m128 x64 = _mm_add_ps(x128, _mm_movehl_ps(x128, x128));
    /* ( -, -, -, x0+x1+x2+x3+x4+x5+x6+x7 ) */
    const __m128 x32 = _mm_add_ss(x64, _mm_shuffle_ps(x64, x64, 0x55));
    /* Conversion to float is a no-op on x86-64 */
    return _mm_cvtss_f32(x32);
}

void linear::matrix_mul(const float *matA, const float *matB, float *matR) {
    // TODO two versions - cpu cache-unfriendly + friendly
    for (int y = 0; y < MATRIX_SIZE; y++) {
        for (int x = 0; x < MATRIX_SIZE; x++) {
            float num = 0;

            for (int n = 0; n < MATRIX_SIZE; n++) {
                num += matA[y*MATRIX_SIZE+n] * matB[n*MATRIX_SIZE+x];
            }

            matR[y*MATRIX_SIZE+x] = num;
        }
    }
}

void avx::matrix_mul(const float *matA, const float *matB, float *matR) {
    for (int y = 0; y < MATRIX_SIZE; y++) {
        for (int x = 0; x < MATRIX_SIZE; x++) {
            __m256 a = _mm256_load_ps(&matA[y*MATRIX_SIZE]);
            __m256 b = _mm256_load_ps(&matB[x*MATRIX_SIZE]);
            __m256 r = _mm256_mul_ps(a, b);

            matR[y*MATRIX_SIZE+x] = _mm256_reduce_add_ps(r);
        }
    }
}