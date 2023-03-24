
set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "-Wall -Wextra -pedantic -fdeclspec")

# Scalar
#=========

add_library(neonbench_cpu_scalar SHARED scalar.cpp)
set_target_properties(neonbench_cpu_scalar PROPERTIES
    POSITION_INDEPENDENT_CODE ON
)
target_compile_definitions(neonbench_cpu_scalar PRIVATE)

# SSE
#=======

add_library(neonbench_cpu_sse SHARED sse.cpp)
set_target_properties(neonbench_cpu_sse PROPERTIES
    POSITION_INDEPENDENT_CODE ON
)
target_compile_definitions(neonbench_cpu_sse PRIVATE
    -D__SSE__
)
target_compile_options(neonbench_cpu_sse PRIVATE
    -msse
)

# AVX
#=======

add_library(neonbench_cpu_avx SHARED avx.cpp)
set_target_properties(neonbench_cpu_avx PROPERTIES
    POSITION_INDEPENDENT_CODE ON
)
target_compile_definitions(neonbench_cpu_avx PRIVATE
    -D__AVX__
)
target_compile_options(neonbench_cpu_avx PRIVATE
    -mavx
)

# AVX512f
#===========

add_library(neonbench_cpu_avx512f SHARED avx512f.cpp)
set_target_properties(neonbench_cpu_avx512f PROPERTIES
    POSITION_INDEPENDENT_CODE ON
)
target_compile_definitions(neonbench_cpu_avx512f PRIVATE
    -D__AVX512F__
)
target_compile_options(neonbench_cpu_avx512f PRIVATE
    -mavx512f
)
