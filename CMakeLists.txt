cmake_minimum_required(VERSION 3.13)
project(neonbench)

set(CMAKE_C_COMPILER clang)
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_FLAGS "-Wall -Wextra")

set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-Wall -Wextra")
# -fdeclspec

find_package(OpenCL)
find_package(CUDA)

# build CPU libs
add_subdirectory(cpu)

# check if OpenCL is available
if(OpenCL_FOUND)
    add_compile_definitions(HAVE_OPENCL=1)
    message("OpenCL found, building with OpenCL support.")
else()
    message("OpenCL not found, building without OpenCL support.")
endif()

# check if CUDA is available
if(CUDA_FOUND)
    add_compile_definitions(HAVE_CUDA=1)
    message("CUDA found, building with CUDA support.")

    # build CUDA obj
    cuda_add_library(cuda_obj cuda/cuda.cu)
    set(CUDA_NVCC_FLAGS "")
    set(CMAKE_CUDA_FLAGS "")
    set_target_properties(cuda_obj PROPERTIES
        CUDA_STANDARD 14
        CUDA_STANDARD_REQUIRED ON
        CUDA_EXTENSIONS OFF
        CUDA_SEPARABLE_COMPILATION ON
    )
    target_include_directories(cuda_obj PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/cuda)
else()
    message("CUDA not found, building without CUDA support.")
endif()

# build main bin
add_executable(${PROJECT_NAME} benchmark.cpp)

include_directories(${CMAKE_SOURCE_DIR}/cpu)

# link libraries
target_link_libraries(${PROJECT_NAME}
    pthread
    dl
    neonbench_cpu_scalar
    neonbench_cpu_sse
    neonbench_cpu_avx
    neonbench_cpu_avx512f
)

# link OpenCL library if available
if(OpenCL_FOUND)
    target_link_libraries(${PROJECT_NAME}
        OpenCL
    )
endif()

# link CUDA library if available
if(CUDA_FOUND)
    target_link_libraries(${PROJECT_NAME}
        cuda_obj
    )
endif()
