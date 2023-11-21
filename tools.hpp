
#include <string>
#include <sstream>
#include <math.h>
#include <iostream>
#include <chrono>
#include <vector>
#include <thread>
#include <algorithm>

#include "benchmarked-object-float.hpp"

// runtime library loading
#ifdef _WIN32
    #include <windows.h>
    #include <stdio.h>
#elif __APPLE__ || __linux__
    #include <dlfcn.h>
#endif

typedef void (*VECTOR_ADD) (const size_t, const float*, const float*, float*);

void num_si_prefix(int64_t n, std::string *numStr) {
    std::stringstream ss;

    if (n > pow(10, 10))
        ss << (int)(n / pow(10, 9)) << " G";
    else if (n > pow(10, 7))
        ss << (int)(n / pow(10, 6)) << " M";
    else if (n > pow(10, 4))
        ss << (int)(n / pow(10, 3)) << " k";
    else
        ss << (int)n;

    *numStr = ss.str();
}

void num_bin_prefix(int64_t n, std::string *numStr) {
    std::stringstream ss;

    if (n > pow(10, 10))
        ss << (int)(n / pow(2, 30)) << " Gi";
    else if (n > pow(10, 7))
        ss << (int)(n / pow(2, 20)) << " Mi";
    else if (n > pow(10, 4))
        ss << (int)(n / pow(2, 10)) << " ki";
    else
        ss << (int)n << " ";

    *numStr = ss.str();
}

int64_t compute_points(unsigned matSize, unsigned testIter, int64_t time, std::string *points) {
    double p = (1.0 / ((double)time*pow(10, -6))) * matSize * testIter;
    num_si_prefix(p, points);

    return p;
}

void reset_result_matrix(BenchmarkedObjectFloat &bo) {
    for (unsigned i = 0; i < bo.vectorSize; ++i)
        bo.vecR[i] = 0;
}

bool test_benchmark(BenchmarkedObjectFloat &bo) {
    alignas(64) float matRef[bo.vectorSize];
    for (unsigned i = 0; i < bo.vectorSize; i++) {
        matRef[i] = bo.vecA[i] + bo.vecB[i];
    }

    for (unsigned i = 0; i < bo.vectorSize; ++i) {
        if (bo.vecR[i] != matRef[i]) {
            reset_result_matrix(bo);
            return false;
        }
    }

    reset_result_matrix(bo);
    return true;
}

void make_benchmark_thread(
        VECTOR_ADD vectorAdd,
        unsigned matSize,
        unsigned testIter,
        const float *matA,
        const float *matB,
        float *matR) {
    for (unsigned i = 0; i < testIter; i++) {
        vectorAdd(matSize, matA, matB, matR);
    }
}

int64_t make_benchmark(
        const char *libName,
        unsigned parallel,
        BenchmarkedObjectFloat &bo) {
    // https://youtu.be/_kIa4D7kQ8I

    // Linux + MacOS
    // dlopen()
    // dlsym()
    // dlclose()

    // Windows
    // LoadLibrary() / LoadLibraryEX()
    // GetProcAddress()
    // FreeLibrary()

    // Linux
    // lib.so

    // MacOS
    // lib.dylib

    // Windows
    // lib.dll


    // Load library
    #ifdef _WIN32
        HINSTANCE handle = LoadLibrary(TEXT(libName));
        if (handle == NULL) {
            perror("Could not load the dynamic library");
            exit(-1);
        }
    #elif __APPLE__ || __linux__
        void *handle = dlopen(libName, RTLD_LAZY); // RTLD_NOW
        if (handle == NULL) {
            std::cerr << dlerror() << std::endl;
            exit(-1);
        }
    #endif

    // Get vector_add function
    #ifdef _WIN32
        VECTOR_ADD vector_add = (VECTOR_ADD)GetProcAddress(handle, "vector_add");
        if (vector_add == NULL) {
            perror("Could not locate the function in the library");
            exit(-2);
        }
    #elif __APPLE__ || __linux__
        VECTOR_ADD vector_add = (VECTOR_ADD)dlsym(handle, "vector_add");
        if (vector_add == NULL) {
            std::cerr << dlerror() << std::endl;
            exit(-2);
        }
    #endif

    // Make benchmark
    auto s = std::chrono::high_resolution_clock::now();

    std::vector<std::thread> th(parallel);
    for (unsigned i = 0; i < parallel; ++i) {
        unsigned fragmentSize = (int)(bo.vectorSize / parallel);
        unsigned iStart = i * fragmentSize;
        unsigned iSize = fragmentSize;//std::min((i+1)*fragmentSize, bo.vectorSize) - iStart;
        th[i] = std::thread(make_benchmark_thread, vector_add, iSize, bo.iterations, &bo.vecA[iStart], &bo.vecB[iStart], &bo.vecR[iStart]);
    }

    for (unsigned i = 0; i < parallel; ++i) {
        th[i].join();
    }

    auto e = std::chrono::high_resolution_clock::now();
    auto t = std::chrono::duration_cast<std::chrono::microseconds>(e - s);

    // Close library
    #ifdef _WIN32
        FreeLibrary(handle);
    #elif __APPLE__ || __linux__
        dlclose(handle);
    #endif

    return t.count();
}
