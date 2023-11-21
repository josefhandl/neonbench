# Neonbench

An attempt to create a CPU and GPU benchmark while learning the basics of GPU computing platforms and cross compilation.

## Requirements

Ubuntu:
```
# necessary:
cmake make clang libstdc++-12-dev

# optional (CUDA):
# TODO cuda runtime

# optional (OpenCL):
# TODO opencl runtime for intel/amd cpu, nvidia/amd gpu
ocl-icd-opencl-dev # OpenCL headers and libraries
intel-opencl-icd # Intel GPU runtime
mesa-opencl-icd # Mesa runtime
```

## Usage

Compile:
```
cmake .
make
```

Run:
```
./neonbench
```
