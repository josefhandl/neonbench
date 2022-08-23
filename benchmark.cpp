#include <chrono>
#include <vector>
#include <iostream>
#include <string.h> // memory
#include <cmath>
#include <sstream>

#ifdef _WIN32
#elif __APPLE__
    #include <unistd.h> // cpu cores
    #include <sys/sysctl.h> // get memory size
#else
    #include <unistd.h> // cpu cores
    #include <sys/sysinfo.h> // get memory size
#endif

// Get OS info
#ifdef _WIN32
#else
    #include <sys/utsname.h>
#endif

#ifdef _WIN32
    #define LIB_SSE "sse.dll"
    #define LIB_AVX "avx.dll"
    #define LIB_AVX512 "avx512f.dll"
#elif __APPLE__
    #define LIB_SSE "sse.dylib"
    #define LIB_AVX "avx.dylib"
    #define LIB_AVX512 "avx512f.dylib"
#else
    #define LIB_SSE "./sse.so"
    #define LIB_AVX "./avx.so"
    #define LIB_AVX512 "./avx512f.so"
#endif

// runtime library loading
#ifdef _WIN32
    #include <windows.h>
    #include <stdio.h>
#else
    #include <dlfcn.h>
#endif

#ifdef _WIN32
    //  Windows
    #include <intrin.h>
    void cpuid(int info[4], int x) {
        //__cpuidex(info, x, 0);
        __cpuid(info, x);
    }
    //#define cpuid(info, x)    __cpuidex(info, x, 0)
#else
    //  GCC Intrinsics
    #include <cpuid.h>
    void cpuid(int info[4], int InfoType) {
        __cpuid_count(InfoType, 0, info[0], info[1], info[2], info[3]);
    }
#endif

bool HW_SSE = false;
bool HW_AVX = false;
bool HW_AVX2 = false;
bool HW_AVX512F = false;

#define TEST_ITERATIONS 16384*8
#define MAX_RAND_NUM 16u
#define AVX_WIDTH_FLOAT (int)(256/(sizeof(float)*8))
#define AVX512_WIDTH_FLOAT AVX_WIDTH_FLOAT*2
#define MATRIX_SIZE (1*AVX512_WIDTH_FLOAT)
#define MATRIX_SIZE_FULL MATRIX_SIZE*MATRIX_SIZE

// init matrices
alignas(64) float matA[MATRIX_SIZE_FULL];
alignas(64) float matB[MATRIX_SIZE_FULL];
alignas(64) float matBT[MATRIX_SIZE_FULL];
alignas(64) float matRf[MATRIX_SIZE_FULL];
alignas(64) float matR[MATRIX_SIZE_FULL];


void vector_add_scalar(const float *matA, const float *matB, float *matR) {
    for (size_t i = 0; i < MATRIX_SIZE_FULL; i++) {
        matR[i] = matA[i] + matB[i];
    }
}

void init_matrices() {
    // fill matrices
    // TODO cache-friendly?
    for (int y = 0; y < MATRIX_SIZE; y++) {
        for (int x = 0; x < MATRIX_SIZE; x++) {
            matA[y*MATRIX_SIZE+x] = static_cast<float>(rand() % MAX_RAND_NUM);

            float b = static_cast<float>(rand() % MAX_RAND_NUM);
            matB[y*MATRIX_SIZE+x] = b;
            matBT[x*MATRIX_SIZE+y] = b;
        }
    }
}

void reset_result_matrix(float *matR) {
    for (int y = 0; y < MATRIX_SIZE; y++) {
        for (int x = 0; x < MATRIX_SIZE; x++) {
            matR[y*MATRIX_SIZE+x] = 0;
        }
    }
}

bool check_vm(std::string *vmName) {
    // https://stackoverflow.com/questions/41750144/c-how-to-detect-the-virtual-machine-your-application-is-running-in-has-focus
    int cpuInfo[4] = {};

    //
    // Upon execution, code should check bit 31 of register ECX
    // (the “hypervisor present bit”). If this bit is set, a hypervisor is present.
    // In a non-virtualized environment, the bit will be clear.
    //
    cpuid(cpuInfo, 1);
    

    if (!(cpuInfo[2] & (1 << 31)))
        return false;
    
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

    static const VendorIdStr vendors[]{
    "KVMKVMKVM\0\0\0", // KVM 
    "Microsoft Hv",    // Microsoft Hyper-V or Windows Virtual PC */
    "VMwareVMware",    // VMware 
    "XenVMMXenVMM",    // Xen 
    "prl hyperv  ",    // Parallels
    "VBoxVBoxVBox"     // VirtualBox 
    };

    for (const auto& vendor : vendors)
    {
        if (!memcmp(vendor, hyperVendorId, vendorIdLength)) {
            *vmName = std::string(vendor);
            return true;
        }
    }

    return false;
}

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

void compute_points(int64_t time, std::string *points) {
    double p = (1.0 / ((double)time*pow(10, -6))) * MATRIX_SIZE_FULL * TEST_ITERATIONS;

    num_si_prefix(p, points);
}

void get_cpu_info() {
    // https://stackoverflow.com/questions/850774/how-to-determine-the-hardware-cpu-and-ram-on-a-machine
    char CPUBrandString[0x40];
    int CPUInfo[4] = {0,0,0,0};

    cpuid(CPUInfo, 0x80000000);
    unsigned int nExIds = CPUInfo[0];

    memset(CPUBrandString, 0, sizeof(CPUBrandString));

    for (unsigned int i = 0x80000000; i <= nExIds; ++i) {
        cpuid(CPUInfo, i);

        if (i == 0x80000002)
            memcpy(CPUBrandString, CPUInfo, sizeof(CPUInfo));
        else if (i == 0x80000003)
            memcpy(CPUBrandString + 16, CPUInfo, sizeof(CPUInfo));
        else if (i == 0x80000004)
            memcpy(CPUBrandString + 32, CPUInfo, sizeof(CPUInfo));
    }

    // https://stackoverflow.com/questions/150355/programmatically-find-the-number-of-cores-on-a-machine
    #ifdef _WIN32
    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);
    int numCPU = sysinfo.dwNumberOfProcessors;
    #else
    int numCPU = sysconf(_SC_NPROCESSORS_ONLN);
    #endif

    // https://stackoverflow.com/questions/349889/how-do-you-determine-the-amount-of-linux-system-ram-in-c
    // https://stackoverflow.com/questions/43481494/total-ram-size-linux-sysinfo-vs-proc-meminfo
    #ifdef _WIN32
    unsigned long long totalRam;
    GetPhysicallyInstalledSystemMemory(&totalRam);
    totalRam *= 1024;
    #elif __APPLE__
    int mib[2];
    int64_t totalRam;
    size_t length;
    mib[0] = CTL_HW;
    mib[1] = HW_MEMSIZE;
    length = sizeof(int64_t);
    sysctl(mib, 2, &totalRam, &length, NULL, 0);
    #else
    struct sysinfo sys_info;
    int64_t totalRam = 0;
    if (sysinfo(&sys_info) != -1)
        totalRam = ((uint64_t) sys_info.totalram * sys_info.mem_unit);
    #endif

    std::cout << "System Info:" << std::endl;
    std::cout << "------------------------" << std::endl;

    // https://stackoverflow.com/questions/6315666/c-get-linux-distribution-name-version
    // TODO - posix only
    #ifdef _WIN32
    #else
        struct utsname osInfo;
        uname(&osInfo);
        std::cout << "OS: "
                  << osInfo.sysname << " "
                  << osInfo.nodename << " "
                  << osInfo.release << " "
                  << osInfo.version << " "
                  << osInfo.machine << std::endl;
    #endif

    std::cout << "CPU: " << CPUBrandString << std::endl;
    std::cout << "Cores: " << numCPU << std::endl;
    std::string totalRamStr;
    num_si_prefix(totalRam, &totalRamStr);
    std::cout << "Memory size: " << totalRamStr << "B" << std::endl;
    std::string vmName;
    bool vmPresence = check_vm(&vmName);
    std::cout << "Virtual Machine: " << (vmPresence ? vmName : "False") << std::endl;
    std::cout << std::endl;
    std::cout << "Benchmark:" << std::endl;
    std::cout << "------------------------" << std::endl;
}

void get_cpu_inst_set() {
    //https://stackoverflow.com/questions/6121792/how-to-check-if-a-cpu-supports-the-sse3-instruction-set
    //https://github.com/Mysticial/FeatureDetector
    // check sse, avx and avx512f support
    int info[4];
    cpuid(info, 0);
    int nIds = info[0];

    cpuid(info, 0x80000000);
    //unsigned nExIds = info[0];

    if (nIds >= 0x00000001){
        cpuid(info,0x00000001);
        HW_SSE = (info[3] & ((int)1 << 25)) != 0;
        HW_AVX = (info[2] & ((int)1 << 28)) != 0;
    }

    if (nIds >= 0x00000007){
        cpuid(info,0x00000007);
        HW_AVX2    = (info[1] & ((int)1 <<  5)) != 0;
        HW_AVX512F = (info[1] & ((int)1 << 16)) != 0;
    }
}

int64_t make_benchmark(const char *libName) {
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

    #ifdef _WIN32
        HINSTANCE handle = LoadLibrary(TEXT(libName));
        if (handle == NULL) {
            perror("Could not load the dynamic library");
            exit(-1);
        }
    #else
        void *handle = dlopen(libName, RTLD_LAZY); // RTLD_NOW
        if (handle == NULL) {
            std::cerr << dlerror() << std::endl;
            exit(-1);
        }
    #endif

    #ifdef _WIN32
        typedef void (*VADD) (const size_t, const float*, const float*, float*);
        VADD vector_add = (VADD)GetProcAddress(handle, "vector_add");
        if (vector_add == NULL) {
            perror("Could not locate the function in the library");
            exit(-2);
        }
    #else
    void (*vector_add) (const size_t, const float*, const float*, float*);
        vector_add = (void (*)(const size_t, const float*, const float*, float*))dlsym(handle, "vector_add");
        if (vector_add == NULL) {
            std::cerr << dlerror() << std::endl;
            exit(-2);
        }
    #endif

    auto s = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < TEST_ITERATIONS; i++) {
        vector_add(MATRIX_SIZE_FULL, matA, matB, matR);
    }

    auto e = std::chrono::high_resolution_clock::now();
    auto t = std::chrono::duration_cast<std::chrono::microseconds>(e - s);

    #ifdef _WIN32
        FreeLibrary(handle);
    #else
        dlclose(handle);
    #endif

    return t.count();
}

int main() {

    init_matrices();

    get_cpu_info();

    get_cpu_inst_set();

    std::string points;

    // Scalar
    //---------
    auto s = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < TEST_ITERATIONS; i++) {
        vector_add_scalar(matA, matB, matR);
    }
    auto e = std::chrono::high_resolution_clock::now();
    auto t = std::chrono::duration_cast<std::chrono::microseconds>(e - s);
    compute_points(t.count(), &points);
    std::cout << "Scalar: " << points << std::endl;

    // SSE
    //---------
    std::cout << "SSE:    ";
    if (HW_SSE) {
        compute_points(make_benchmark(LIB_SSE), &points);
        std::cout << points;
    } else
        std::cout << "Not supported";
    std::cout << std::endl;

    // AVX
    //---------
    std::cout << "AVX:    ";
    if (HW_AVX) {
        compute_points(make_benchmark(LIB_AVX), &points);
        std::cout << points;
    } else
        std::cout << "Not supported";
    std::cout << std::endl;

    // AVX512
    //---------
    std::cout << "AVX512: ";
    if (HW_AVX512F) {
        compute_points(make_benchmark(LIB_AVX512), &points);
        std::cout << points;
    } else
        std::cout << "Not supported";
    std::cout << std::endl;
}
