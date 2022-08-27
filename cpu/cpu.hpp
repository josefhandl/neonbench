
#include <chrono>
#include <iostream>
#include <string.h> // memory
#include <string>
#include <map>

#include "../tools.hpp"

#ifdef _WIN32
    #define LIB_SSE "sse.dll"
    #define LIB_AVX "avx.dll"
    #define LIB_AVX512 "avx512f.dll"
#elif __APPLE__
    #define LIB_SSE "sse.dylib"
    #define LIB_AVX "avx.dylib"
    #define LIB_AVX512 "avx512f.dylib"
#elif __linux__
    #define LIB_SSE "./cpu/sse.so"
    #define LIB_AVX "./cpu/avx.so"
    #define LIB_AVX512 "./cpu/avx512f.so"
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

class ModuleCpu {

private:
    bool hw_sse = false;
    bool hw_avx = false;
    //bool hw_avx2 = false;
    bool hw_avx512f = false;

    std::string cpuName;
    unsigned cpuCores;
    unsigned long long ramSize;

    bool vmDetected = false;
    std::string vmName;

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

    void inspect_ram_size() {
        // https://stackoverflow.com/questions/349889/how-do-you-determine-the-amount-of-linux-system-ram-in-c
        // https://stackoverflow.com/questions/43481494/total-ram-size-linux-sysinfo-vs-proc-meminfo
        // https://ideone.com/RuHMUK
        #ifdef _WIN32
            GetPhysicallyInstalledSystemMemory(&ramSize);
            ramSize *= 1024;
        #elif __APPLE__
            int mib[2];
            size_t length;
            mib[0] = CTL_HW;
            mib[1] = HW_MEMSIZE;
            length = sizeof(int64_t);
            sysctl(mib, 2, &ramSize, &length, NULL, 0);
        #elif __linux__
            struct sysinfo sys_info;
            if (sysinfo(&sys_info) != -1)
                ramSize = ((uint64_t) sys_info.totalram * sys_info.mem_unit);
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

        const int vendorIdLength = 13;
        using VendorIdStr = char[vendorIdLength];

        VendorIdStr hyperVendorId = {};

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
            if (!memcmp(vendor.first, hyperVendorId, vendorIdLength)) {
                vmDetected = true;
                vmName = std::string(vendor.second);
            }
        }
    }

    void vector_add_scalar(unsigned matSize, const float *matA, const float *matB, float *matR) {
        for (unsigned i = 0; i < matSize; i++) {
            matR[i] = matA[i] + matB[i];
        }
    }

public:
    void inspect() {
        inspect_inst_sets();
        inspect_cpu_name();
        inspect_cpu_cores();
        inspect_ram_size();
        inspect_hypervisor_presence();
    }

    void printInfo() {
        std::cout << "CPU info:" << std::endl;
        std::cout << "--------------------------------------" << std::endl;
        std::cout << "CPU Name: " << cpuName << std::endl;
        std::cout << "CPUs (threads): " << cpuCores << std::endl;

        std::string ramSizeStr;
        num_si_prefix(ramSize, &ramSizeStr);
        std::cout << "RAM size: " << ramSizeStr << "B" << std::endl;

        std::cout << "Virtual Machine: ";
        if (vmDetected)
            std::cout << "Hypervisor detected - " << vmName;
        else
            std::cout << "Hypervisor not detected";
        std::cout << std::endl;

        std::cout << std::endl;
    }

    void benchmark(unsigned matSize, unsigned testIter, const float *matA, const float *matB, float *matR) {
        std::cout << "CPU Benchmark:" << std::endl;
        std::cout << "--------------------------------------" << std::endl;

        std::string points;

        // Scalar
        //---------
        auto s = std::chrono::high_resolution_clock::now();
        for (unsigned i = 0; i < testIter; i++) {
            vector_add_scalar(matSize, matA, matB, matR);
        }
        auto e = std::chrono::high_resolution_clock::now();
        auto t = std::chrono::duration_cast<std::chrono::microseconds>(e - s);
        compute_points(matSize, testIter, t.count(), &points);

        bool scalar_ok = test_benchmark(matSize, matA, matB, matR);
        std::cout << "Scalar: " << (scalar_ok ? points : "Failed") << std::endl;

        // SSE
        //---------
        std::cout << "SSE:    ";
        if (hw_sse) {
            compute_points(matSize, testIter, make_benchmark(LIB_SSE, matSize, testIter, matA, matB, matR), &points);

            bool sse_ok = test_benchmark(matSize, matA, matB, matR);
            std::cout << (sse_ok ? points : "Failed");
        } else
            std::cout << "Not supported";
        std::cout << std::endl;

        // AVX
        //---------
        std::cout << "AVX:    ";
        if (hw_avx) {
            compute_points(matSize, testIter, make_benchmark(LIB_AVX, matSize, testIter, matA, matB, matR), &points);

            bool avx_ok = test_benchmark(matSize, matA, matB, matR);
            std::cout << (avx_ok ? points : "Failed");
        } else
            std::cout << "Not supported";
        std::cout << std::endl;

        // AVX512
        //---------
        std::cout << "AVX512: ";
        if (hw_avx512f) {
            compute_points(matSize, testIter, make_benchmark(LIB_AVX512, matSize, testIter, matA, matB, matR), &points);

            bool avx512_ok = test_benchmark(matSize, matA, matB, matR);
            std::cout << (avx512_ok ? points : "Failed");
        } else
            std::cout << "Not supported";
        std::cout << std::endl;

        std::cout << std::endl;
    }
};
