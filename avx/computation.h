
#ifndef __COMPUTATION_H__
#define __COMPUTATION_H__

class Computation {
public:
    Computation();

    virtual void vector_add(const size_t matSize, const float *matA, const float *matB, float *matR);
};

#endif
