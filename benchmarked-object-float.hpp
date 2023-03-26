
#pragma once

#include <vector>
#include <stdlib.h>

#define MAX_RAND_NUM 16u

class BenchmarkedObjectFloat {
public:
    float* vecA;
    float* vecB;
    float* vecR;
    unsigned vectorSize;
    unsigned iterations;

    BenchmarkedObjectFloat(unsigned vectorSize, unsigned iterations, unsigned maxRnd)
            : vectorSize(vectorSize), iterations(iterations) {
        this->vecA = static_cast<float*>(aligned_alloc(64, vectorSize * sizeof(float)));
        this->vecB = static_cast<float*>(aligned_alloc(64, vectorSize * sizeof(float)));
        this->vecR = static_cast<float*>(aligned_alloc(64, vectorSize * sizeof(float)));

        for (unsigned i = 0; i < vectorSize; ++i) {
            vecA[i] = static_cast<float>(rand() % maxRnd);
            vecB[i] = static_cast<float>(rand() % maxRnd);
        }
    }

    BenchmarkedObjectFloat(unsigned vectorSize, unsigned iterations)
            : BenchmarkedObjectFloat(vectorSize, iterations, MAX_RAND_NUM) {}
};
