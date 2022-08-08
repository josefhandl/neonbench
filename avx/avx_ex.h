#pragma once

#include <immintrin.h>
#include <chrono>
#include <vector>
#include <iostream>

namespace avx256 {
    void vector_add(const float*, const float*, float*);
    void matrix_mul(const float*, const float*, float*);
}

namespace avx128 {
    void vector_add(const float*, const float*, float*);
}

namespace linear {
    void vector_add(const float*, const float*, float*);
    void matrix_mul(const float*, const float*, float*);
}