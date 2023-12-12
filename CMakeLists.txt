cmake_minimum_required(VERSION 3.13)
project(neonbench)

set(CMAKE_C_COMPILER clang)
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_FLAGS "-Wall -Wextra -fdeclspec")

set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-Wall -Wextra -fdeclspec")

# Set the vcpkg toolchain file
set(CMAKE_TOOLCHAIN_FILE ./vcpkg/scripts/buildsystems/vcpkg.cmake)

# Find OpenCL and CUDA
find_package(OpenCL)
find_package(CUDA)

# Build own CPU libs
add_subdirectory(cpu)

# Check if OpenCL is available
if(OpenCL_FOUND)
    if(WIN32)
        include_directories(./vcpkg/packages/opencl_x64-windows/include)
        link_directories(./vcpkg/packages/opencl_x64-windows/lib)
    endif()
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

# Link own CPU lib
target_link_libraries(${PROJECT_NAME}
    neonbench_cpu_scalar
)

# Link own architecture specific CPU libs
if (CMAKE_SYSTEM_PROCESSOR MATCHES "arm*|aarch64*")
    target_link_libraries(${PROJECT_NAME}
        neonbench_cpu_neon
        neonbench_cpu_sve
    )
elseif (CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64*|x86*")
    target_link_libraries(${PROJECT_NAME}
        neonbench_cpu_sse
        neonbench_cpu_avx
        neonbench_cpu_avx512f
    )
endif()

# Link pthread library
if(WIN32)
    target_link_libraries(${PROJECT_NAME}
        ./vcpkg/installed/x64-windows/lib/pthreadVC3
    )
else()
    target_link_libraries(${PROJECT_NAME}
        pthread
    )
endif()

# Link OpenCL library if available
if(OpenCL_FOUND)
    target_link_libraries(${PROJECT_NAME}
        OpenCL
    )
endif()

# Link CUDA library if available
if(CUDA_FOUND)
    target_link_libraries(${PROJECT_NAME}
        cuda_obj
    )
endif()
