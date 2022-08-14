#pragma once

namespace sse {
    bool vector_add(const float*, const float*, float*);
}

namespace avx {
    bool vector_add(const float*, const float*, float*);
    bool matrix_mul(const float*, const float*, float*);
}

namespace avx512 {
    bool vector_add(const float*, const float*, float*);
}

namespace linear {
    bool vector_add(const float*, const float*, float*);
    bool matrix_mul(const float*, const float*, float*);
}
