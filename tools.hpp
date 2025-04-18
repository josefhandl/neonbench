#ifndef NEONBENCH_TOOLS_HPP
#define NEONBENCH_TOOLS_HPP

#include "benchmarked-object-float.hpp"

void num_si_prefix(int64_t n, std::string *numStr);
void num_bin_prefix(int64_t n, std::string *numStr);
int64_t compute_points(unsigned int a, unsigned int b, long c, std::string *result);
void reset_result_matrix(BenchmarkedObjectFloat& obj);
bool test_benchmark(BenchmarkedObjectFloat& obj);
void make_benchmark_thread(void (*func)(unsigned long, const float*, const float*, float*), unsigned int a, unsigned int b, const float* c, const float* d, float* e);
int64_t make_benchmark(const char* name, unsigned int a, BenchmarkedObjectFloat& obj);

#endif // NEONBENCH_TOOLS_HPP