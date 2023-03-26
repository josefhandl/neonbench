
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
        this->vecA = static_cast<int*>(aligned_alloc(64, vectorSize * sizeof(float)));
        this->vecB = static_cast<int*>(aligned_alloc(64, vectorSize * sizeof(float)));
        this->vecR = static_cast<int*>(aligned_alloc(64, vectorSize * sizeof(float)));

        for (unsigned i = 0; i < vectorSize; ++i) {
            vecA[i] = static_cast<int>(rand() % maxRnd);
            vecB[i] = static_cast<int>(rand() % maxRnd);
        }
    }

    BenchmarkedObjectInt(unsigned vectorSize, unsigned iterations)
            : BenchmarkedObjectInt(vectorSize, iterations, MAX_RAND_NUM) {}

};
