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
ocl-icd-opencl-dev opencl-clhpp-headers # OpenCL headers and libraries
intel-opencl-icd # Intel GPU runtime
mesa-opencl-icd # Mesa runtime
```

Windows:
```
TODO clang cmake make

# Install Vcpkg
# https://github.com/microsoft/vcpkg

# Install required packages
./vcpkg.exe --triplet=x64-windows install pthread

# Install OpenCL (optional)
./vcpkg.exe --triplet=x64-windows install opencl
```

## Usage

Compile:
```
cmake .
make
```

Run:
```
# Linux
./neonbench

# Windows
./neonbench.exe
```
