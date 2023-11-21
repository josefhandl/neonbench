
#pragma once

#include <vector>
#include <stdlib.h>

#define MAX_RAND_NUM 16u

class BenchmarkedObjectInt {
public:
    int* vecA;
    int* vecB;
    int* vecR;
    unsigned vectorSize;
    unsigned iterations;

    BenchmarkedObjectInt(unsigned vectorSize, unsigned iterations, unsigned maxRnd)
            : vectorSize(vectorSize), iterations(iterations) {
        #ifdef _WIN32
        this->vecA = static_cast<int*>(_aligned_malloc(vectorSize * sizeof(int), 64));
        this->vecB = static_cast<int*>(_aligned_malloc(vectorSize * sizeof(int), 64));
        this->vecR = static_cast<int*>(_aligned_malloc(vectorSize * sizeof(int), 64));
        #elif __APPLE__ || __linux__
        this->vecA = static_cast<int*>(aligned_alloc(64, vectorSize * sizeof(int)));
        this->vecB = static_cast<int*>(aligned_alloc(64, vectorSize * sizeof(int)));
        this->vecR = static_cast<int*>(aligned_alloc(64, vectorSize * sizeof(int)));
        #endif

        for (unsigned i = 0; i < vectorSize; ++i) {
            vecA[i] = static_cast<int>(rand() % maxRnd);
            vecB[i] = static_cast<int>(rand() % maxRnd);
        }
    }

    BenchmarkedObjectInt(unsigned vectorSize, unsigned iterations)
            : BenchmarkedObjectInt(vectorSize, iterations, MAX_RAND_NUM) {}

};
