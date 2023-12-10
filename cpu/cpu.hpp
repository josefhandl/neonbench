
#include <chrono>
#include <iostream>
#include <string.h> // memory
#include <string>
#include <map>
#include <memory>

#include "../tools.hpp"
#include "../benchmarked-object-float.hpp"

#ifdef _WIN32
    #define LIB_SCALAR ".\\cpu\\neonbench_cpu_scalar.dll"
    #define LIB_SSE ".\\cpu\\neonbench_cpu_sse.dll"
    #define LIB_AVX ".\\cpu\\neonbench_cpu_avx.dll"
    #define LIB_AVX512 ".\\cpu\\neonbench_cpu_avx512f.dll"
    #define LIB_NEON ".\\cpu\\neonbench_cpu_neon.dll"
#elif __APPLE__
    #define LIB_SCALAR "./cpu/scalar.dylib"
    #define LIB_SSE "./cpu/sse.dylib"
    #define LIB_AVX "./cpu/avx.dylib"
    #define LIB_AVX512 "./cpu/avx512f.dylib"
    #define LIB_NEON "./cpu/neon.dylib"
#elif __linux__
    #define LIB_SCALAR "./cpu/libneonbench_cpu_scalar.so"
    #define LIB_SSE "./cpu/libneonbench_cpu_sse.so"
    #define LIB_AVX "./cpu/libneonbench_cpu_avx.so"
    #define LIB_AVX512 "./cpu/libneonbench_cpu_avx512f.so"
    #define LIB_NEON "./cpu/libneonbench_cpu_neon.so"
#endif

#ifdef _WIN32
    #define NEONBENCH_OS 2
#elif __APPLE__
    #define NEONBENCH_OS 1
#elif __linux__
    #define NEONBENCH_OS 0
#endif

#ifdef __i386__ || __x86_64__
    #define NEONBENCH_ARCH 0
#elif __arm__ || __aarch64__
    #define NEONBENCH_ARCH 1
#endif

#ifdef _WIN32
    //  Windows
    #include <intrin.h>
    void cpuid(int cpuInfo[4], int x) {
        __cpuid(cpuInfo, x);
    }
#elif __APPLE__ || __linux__
    //  GCC Intrinsics
    #include <cpuid.h>
    void cpuid(int cpuInfo[4], int InfoType) {
        __cpuid_count(InfoType, 0, cpuInfo[0], cpuInfo[1], cpuInfo[2], cpuInfo[3]);
    }
#endif

#ifdef _WIN32
    // No additional libs required
#elif __APPLE__
    #include <unistd.h> // cpu cores
    #include <sys/sysctl.h> // get memory size
#elif __linux__
    #include <unistd.h> // cpu cores
    #include <sys/sysinfo.h> // get memory size
#endif

#define HYPER_VENDOR_ID_LENGTH 13

class ModuleCpu {

private:
    std::unique_ptr<BenchmarkedObjectFloat> bo_singleThread;
    std::unique_ptr<BenchmarkedObjectFloat> bo_multiThread;

    bool hw_sse = false;
    bool hw_avx = false;
    //bool hw_avx2 = false;
    bool hw_avx512f = false;

    std::string cpuName;
    unsigned cpuCores;

    bool vmDetected = false;
    std::string vmName;
    char hyperVendorId[HYPER_VENDOR_ID_LENGTH] = {};

    int os = NEONBENCH_OS;
    int architecture = NEONBENCH_ARCH;

    void inspect_inst_sets() {
        // check sse, avx and avx512f support

        //https://stackoverflow.com/questions/6121792/how-to-check-if-a-cpu-supports-the-sse3-instruction-set
        //https://github.com/Mysticial/FeatureDetector

        int cpuInfo[4];
        cpuid(cpuInfo, 0);
        int nIds = cpuInfo[0];

        cpuid(cpuInfo, 0x80000000);
        //unsigned nExIds = cpuInfo[0]; // TODO github, feature detector

        if (nIds >= 0x00000001){
            cpuid(cpuInfo,0x00000001);
            hw_sse = (cpuInfo[3] & ((int)1 << 25)) != 0;
            hw_avx = (cpuInfo[2] & ((int)1 << 28)) != 0;
        }

        if (nIds >= 0x00000007){
            cpuid(cpuInfo,0x00000007);
            //hw_avx2    = (cpuInfo[1] & ((int)1 <<  5)) != 0;
            hw_avx512f = (cpuInfo[1] & ((int)1 << 16)) != 0;
        }
    }

    void inspect_cpu_name() {
        // https://stackoverflow.com/questions/850774/how-to-determine-the-hardware-cpu-and-ram-on-a-machine
        char CPUBrandString[0x40];
        int cpuInfo[4] = {0,0,0,0};

        cpuid(cpuInfo, 0x80000000);
        unsigned int nExIds = cpuInfo[0];

        memset(CPUBrandString, 0, sizeof(CPUBrandString));

        for (unsigned int i = 0x80000000; i <= nExIds; ++i) {
            cpuid(cpuInfo, i);

            if (i == 0x80000002)
                memcpy(CPUBrandString, cpuInfo, sizeof(cpuInfo));
            else if (i == 0x80000003)
                memcpy(CPUBrandString + 16, cpuInfo, sizeof(cpuInfo));
            else if (i == 0x80000004)
                memcpy(CPUBrandString + 32, cpuInfo, sizeof(cpuInfo));
        }

        cpuName = std::string(CPUBrandString);
    }

    void inspect_cpu_cores() {
        // https://stackoverflow.com/questions/150355/programmatically-find-the-number-of-cores-on-a-machine
        #ifdef _WIN32
            SYSTEM_INFO sysinfo;
            GetSystemInfo(&sysinfo);
            cpuCores = sysinfo.dwNumberOfProcessors;
        #elif __APPLE__ || __linux__
            cpuCores = sysconf(_SC_NPROCESSORS_ONLN);
        #endif
    }

    void inspect_hypervisor_presence() {
        // https://stackoverflow.com/questions/41750144/c-how-to-detect-the-virtual-machine-your-application-is-running-in-has-focus
        // https://www.google.com/url?sa=t&rct=j&q=&esrc=s&source=web&cd=&ved=2ahUKEwjGqfaIst_5AhXUtKQKHfjSAWoQFnoECAsQAQ&url=https%3A%2F%2Fwww.duo.uio.no%2Fbitstream%2Fhandle%2F10852%2F51631%2FThesis_Maghsoud.pdf%3Fsequence%3D1&usg=AOvVaw0wPR7iYvbNxC38M3SJ99_8
        int cpuInfo[4] = {};

        //
        // Upon execution, code should check bit 31 of register ECX
        // (the “hypervisor present bit”). If this bit is set, a hypervisor is present.
        // In a non-virtualized environment, the bit will be clear.
        //
        cpuid(cpuInfo, 1);

        if (!(cpuInfo[2] & (1 << 31)))
            return;

        //
        // A hypervisor is running on the machine. Query the vendor id.
        //
        const auto queryVendorIdMagic = 0x40000000;
        cpuid(cpuInfo, queryVendorIdMagic);

        memcpy(hyperVendorId + 0, &cpuInfo[1], 4);
        memcpy(hyperVendorId + 4, &cpuInfo[2], 4);
        memcpy(hyperVendorId + 8, &cpuInfo[3], 4);
        hyperVendorId[12] = '\0';

        const std::map<const char*, const char*> vendors {
            { "KVMKVMKVM\0\0\0", "KVM" },
            { "Microsoft Hv", "Microsoft Hyper-V or Windows Virtual PC" },
            { "VMwareVMware", "VMware" },
            { "XenVMMXenVMM", "Xen" },
            { "prl hyperv  ", "Parallels" },
            { "VBoxVBoxVBox", "VirtualBox" }
        };

        for (const auto& vendor : vendors) {
            if (!memcmp(vendor.first, hyperVendorId, HYPER_VENDOR_ID_LENGTH)) {
                vmDetected = true;
                vmName = std::string(vendor.second);
            }
        }
    }

    void launch_benchmark(const char *libName) {
        std::string points;
        bool benchmark_ok_s;
        bool benchmark_ok_m;

        // Single-thread
        // heat up
        make_benchmark(libName, 1, *bo_singleThread);
        // benchmark
        int64_t points_s = compute_points(bo_singleThread->vectorSize, bo_singleThread->iterations, make_benchmark(libName, 1, *bo_singleThread), &points);
        benchmark_ok_s = test_benchmark(*bo_singleThread);
        std::cout << (benchmark_ok_s ? points : "Invalid result") << "\t";

        // Multi-thread
        // heat up
        make_benchmark(libName, cpuCores, *bo_multiThread);
        // benchmark
        int64_t points_m = compute_points(bo_multiThread->vectorSize, bo_multiThread->iterations, make_benchmark(libName, cpuCores, *bo_multiThread), &points);
        benchmark_ok_m = test_benchmark(*bo_multiThread);
        std::cout << (benchmark_ok_m ? points : "Invalid result");

        if (benchmark_ok_s && benchmark_ok_m) {
            std::cout << "\t" << round((static_cast<double>(points_m) / static_cast<double>(points_s)) * 100.0) / 100.0 << "x";
        }
    }

public:
    void inspect() {
        inspect_inst_sets();
        inspect_cpu_name();
        inspect_cpu_cores();
        inspect_hypervisor_presence();
    }

    void printInfo() {
        std::cout << "CPU info:" << std::endl;
        std::cout << "--------------------------------------" << std::endl;

        std::cout << "Architecture: ";
        if (architecture == 0)
            std::cout << "x86";
        else if (architecture == 1)
            std::cout << "arm";
        std::cout << std::endl;

        std::cout << "OS: ";
        if (os == 2)
            std::cout << "Windows";
        else if (os == 1)
            std::cout << "macOS";
        else if (os == 0)
            std::cout << "Linux";
        std::cout << std::endl;

        std::cout << "CPU Name: " << cpuName << std::endl;
        std::cout << "CPUs (threads): " << cpuCores << std::endl;

        std::cout << "Virtual Machine: ";
        if (vmDetected)
            std::cout << "Hypervisor detected - " << vmName << " (vendor: \"" << hyperVendorId << "\")";
        else
            std::cout << "Hypervisor not detected" << " (vendor: \"" << hyperVendorId << "\")";

        std::cout << std::endl << std::endl;
    }

    void benchmark_prepare(unsigned size, unsigned iterations) {
        bo_singleThread = std::make_unique<BenchmarkedObjectFloat>(size, iterations);
        bo_multiThread = std::make_unique<BenchmarkedObjectFloat>(size*cpuCores, iterations);
    }

    void benchmark() {
        std::cout << "CPU benchmark:" << std::endl;
        std::cout << "--------------------------------------" << std::endl;
        std::cout << "\tST\tMT" << std::endl;

        // Scalar
        //---------
        std::cout << "Scalar: ";
        launch_benchmark(LIB_SCALAR);
        std::cout << std::endl;

        if (architecture == 0) {
            // SSE
            //---------
            std::cout << "SSE:    ";
            if (hw_sse)
                launch_benchmark(LIB_SSE);
            else
                std::cout << "Not supported";
            std::cout << std::endl;

            // AVX
            //---------
            std::cout << "AVX:    ";
            if (hw_avx)
                launch_benchmark(LIB_AVX);
            else
                std::cout << "Not supported";
            std::cout << std::endl;

            // AVX512
            //---------
            std::cout << "AVX512: ";
            if (hw_avx512f)
                launch_benchmark(LIB_AVX512);
            else
                std::cout << "Not supported";
            std::cout << std::endl;
        } else if (architecture == 1) {
            // NEON
            //---------
            std::cout << "NEON: ";
            if (true)
                launch_benchmark(LIB_NEON);
            else
                std::cout << "Not supported";
            std::cout << std::endl;
        }

        std::cout << std::endl;
    }
};
