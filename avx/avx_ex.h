#pragma once

namespace avx {
    void vector_add(const float*, const float*, float*);
    void matrix_mul(const float*, const float*, float*);
}

namespace sse {
    void vector_add(const float*, const float*, float*);
}

namespace linear {
    void vector_add(const float*, const float*, float*);
    void matrix_mul(const float*, const float*, float*);
}