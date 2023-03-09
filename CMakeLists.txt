cmake_minimum_required(VERSION 3.13)
project(neonbench)

set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "-Wall -Wextra -pedantic")
# -fdeclspec

find_package(CUDA REQUIRED)

# build CPU libs
add_subdirectory(cpu)

# build CUDA obj
cuda_add_library(cuda_obj cuda/cuda.cu)
set(CUDA_NVCC_FLAGS "")
set_target_properties(cuda_obj PROPERTIES
    CUDA_STANDARD 14
    CUDA_STANDARD_REQUIRED ON
    CUDA_EXTENSIONS OFF
    CUDA_SEPARABLE_COMPILATION ON
)
target_include_directories(cuda_obj PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/cuda)

# build main bin
add_executable(${PROJECT_NAME} benchmark.cpp)

include_directories(${CMAKE_SOURCE_DIR}/cpu)

# link
target_link_libraries(${PROJECT_NAME}
    pthread
    dl
    OpenCL
    neonbench_cpu_scalar
    neonbench_cpu_sse
    neonbench_cpu_avx
    neonbench_cpu_avx512f
    cuda_obj
)
