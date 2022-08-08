#pragma once

#include <immintrin.h>
#include <chrono>
#include <vector>
#include <iostream>

namespace avx {
    void vector_add(const float*, const float*, float*);
}

namespace linear {
    void vector_add(const float*, const float*, float*);
}