
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

add_library(neonbench_cpu_sse SHARED scalar.cpp)
set_target_properties(neonbench_cpu_sse PROPERTIES
    POSITION_INDEPENDENT_CODE ON
)
target_compile_definitions(neonbench_cpu_sse PRIVATE
    -D__SSE__
)

# AVX
#=======

add_library(neonbench_cpu_avx SHARED scalar.cpp)
set_target_properties(neonbench_cpu_avx PROPERTIES
    POSITION_INDEPENDENT_CODE ON
)
target_compile_definitions(neonbench_cpu_avx PRIVATE
    -D__AVX__
)

# AVX512f
#===========

add_library(neonbench_cpu_avx512f SHARED scalar.cpp)
set_target_properties(neonbench_cpu_avx512f PROPERTIES
    POSITION_INDEPENDENT_CODE ON
)
target_compile_definitions(neonbench_cpu_avx512f PRIVATE
    -D__AVX512F__
)
