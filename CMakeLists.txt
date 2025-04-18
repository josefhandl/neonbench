cmake_minimum_required(VERSION 3.13)
project(neonbench)

set(CMAKE_C_COMPILER clang)
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_FLAGS "-Wall -Wextra")

set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_CXX_STANDARD 17)
#set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "-Wall -Wextra")

# Define colors for unix terminal
if(NOT WIN32)
  string(ASCII 27 Esc)
  set(ColourReset "${Esc}[m")
  set(ColourBold  "${Esc}[1m")
  set(Red         "${Esc}[31m")
  set(Green       "${Esc}[32m")
  set(Yellow      "${Esc}[33m")
  set(Blue        "${Esc}[34m")
  set(Magenta     "${Esc}[35m")
  set(Cyan        "${Esc}[36m")
  set(White       "${Esc}[37m")
  set(BoldRed     "${Esc}[1;31m")
  set(BoldGreen   "${Esc}[1;32m")
  set(BoldYellow  "${Esc}[1;33m")
  set(BoldBlue    "${Esc}[1;34m")
  set(BoldMagenta "${Esc}[1;35m")
  set(BoldCyan    "${Esc}[1;36m")
  set(BoldWhite   "${Esc}[1;37m")
endif()


# Set the vcpkg toolchain file
set(CMAKE_TOOLCHAIN_FILE ./vcpkg/scripts/buildsystems/vcpkg.cmake)

# Find OpenCL, CUDA
find_package(OpenCL)
find_package(CUDA)

# Find HIP
set(CMAKE_THREAD_LIBS_INIT "-lpthread")
find_package(Threads REQUIRED)
set(ROCM_PATH $ENV{ROCM_PATH})
list(APPEND CMAKE_PREFIX_PATH "${ROCM_PATH}/lib/cmake/hip")
find_package(HIP)

# Build own CPU libs
add_subdirectory(cpu)

# Check if OpenCL is available
if(OpenCL_FOUND)
    if(WIN32)
        include_directories(./vcpkg/packages/opencl_x64-windows/include)
        link_directories(./vcpkg/packages/opencl_x64-windows/lib)
    endif()
    add_compile_definitions(HAVE_OPENCL=1)
    message("${BoldGreen}OpenCL found, building with OpenCL support.${ColourReset}")
else()
    message("${BoldYellow}OpenCL not found, building without OpenCL support.${ColourReset}")
endif()

# check if CUDA is available
if(CUDA_FOUND)
    add_compile_definitions(HAVE_CUDA=1)
    message("${BoldGreen}CUDA found, building with CUDA support.${ColourReset}")

    # build CUDA obj
    set(CUDA_HOST_COMPILER "gcc")
    cuda_add_library(cuda_obj cuda/cuda.cu)
else()
    message("${BoldYellow}CUDA not found, building without CUDA support.${ColourReset}")
endif()

if(HIP_FOUND)
    add_compile_definitions(HAVE_HIP=1)
    message("${BoldGreen}HIP found, building with HIP support.${ColourReset}")

    set(CMAKE_CXX_FLAGS "--rocm-path=/opt/rocm ${CMAKE_CXX_FLAGS}")
    set(CMAKE_CXX_COMPILER hipcc)

    set_source_files_properties(hip/hip.hip PROPERTIES LANGUAGE HIP)
    set_source_files_properties(hip/kernel.hip PROPERTIES LANGUAGE HIP)

    enable_language(HIP)

    add_library(hip_obj hip/hip.hip hip/kernel.hip)
    set_target_properties(hip_obj PROPERTIES
        HIP_STANDARD 14
        HIP_STANDARD_REQUIRED ON
        HIP_EXTENSIONS OFF
        HIP_SEPARABLE_COMPILATION ON
    )
    target_include_directories(hip_obj PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/hip)
    target_link_libraries(hip_obj PRIVATE hip::host)
    message("HIP object 'hip_obj' created.")
else()
    message("${BoldYellow}HIP not found, building without HIP support.${ColourReset}")
endif()

# build main bin
add_executable(${PROJECT_NAME} benchmark.cpp tools.cpp)

include_directories(${CMAKE_SOURCE_DIR}/cpu)

# Link own CPU lib
target_link_libraries(${PROJECT_NAME}
    neonbench_cpu_scalar
)

# Link own architecture specific CPU libs
if (CMAKE_SYSTEM_PROCESSOR MATCHES "arm*|aarch64*")
    target_link_libraries(${PROJECT_NAME}
        neonbench_cpu_neon
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
        dl
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

# Link HIP library if available
if(HIP_FOUND)
    target_link_libraries(${PROJECT_NAME}
        hip_obj
	hip::host
	Threads::Threads
    )
endif()
