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

#target_include_directories(${PROJECT_NAME} PUBLIC
#                          "${PROJECT_BINARY_DIR}"
#                          "${PROJECT_SOURCE_DIR}/cpu"
#                          )


#if(CMAKE_SYSTEM_NAME MATCHES Linux)
#    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --target=x86_64-pc-linux")
#    set(CMAKE_CUDA_FLAGS "${CMAKE_CUDA_FLAGS} --gpu-architecture=sm_75")
#endif()

#target_compile_options(benchmark PUBLIC -std=c++14 -Wall -Wextra -pedantic --target=x86_64-pc-linux -fdeclspec)


#install(TARGETS scalar sse avx avx512f
#        LIBRARY DESTINATION lib
#        ARCHIVE DESTINATION lib)
#install(TARGETS benchmark
#        RUNTIME DESTINATION bin)







# add_subdirectory(cpu)
#
#add_library(scalar SHARED cpu/scalar.cpp)
#add_library(sse SHARED cpu/sse.cpp)
#add_library(avx SHARED cpu/avx.cpp)
#add_library(avx512f SHARED cpu/avx512f.cpp)
#add_library(cuda_obj OBJECT cuda/cuda.cu)
#add_executable(benchmark benchmark.cpp $<TARGET_OBJECTS:cuda_obj>)
#
#target_link_libraries(benchmark PRIVATE sse avx avx512f scalar)
#
#if(CMAKE_SYSTEM_NAME MATCHES Linux)
#    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --target=x86_64-pc-linux")
#    set(CMAKE_CUDA_FLAGS "${CMAKE_CUDA_FLAGS} --gpu-architecture=sm_75")
#endif()
#
#target_compile_options(benchmark PUBLIC -std=c++14 -Wall -Wextra -pedantic --target=x86_64-pc-linux -fdeclspec)
#
#install(TARGETS scalar sse avx avx512f
#        LIBRARY DESTINATION lib
#        ARCHIVE DESTINATION lib)
#install(TARGETS benchmark
#        RUNTIME DESTINATION bin)
#