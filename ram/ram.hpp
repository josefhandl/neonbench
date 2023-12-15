
#include <string>

#include "../benchmarked-object-int.hpp"

class ModuleRam {
private:
    std::unique_ptr<BenchmarkedObjectInt> bo;

    unsigned long long ramSize;
    unsigned long long swapSize;

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
            if (sysinfo(&sys_info) != -1) {
                ramSize = (uint64_t)sys_info.totalram * ((double)sys_info.mem_unit * (double)1024 / (double)1000);
                swapSize = sys_info.totalswap;
            }
        #endif
    }

public:
    void inspect() {
        inspect_ram_size();
    }

    void printInfo() {
        std::cout << "RAM info:" << std::endl;
        std::cout << "--------------------------------------" << std::endl;
        std::string ramSizeStr;
        std::string swapSizeStr;
        num_bin_prefix(ramSize, &ramSizeStr);
        num_bin_prefix(swapSize, &swapSizeStr);
        std::cout << "RAM size: " << ramSizeStr << "B" << std::endl;
        std::cout << "Swap size: " << swapSizeStr << "B" << std::endl;

        std::cout << std::endl;
    }

    void benchmark_prepare(unsigned size, unsigned iterations) {
        bo = std::make_unique<BenchmarkedObjectInt>(size, iterations, size);
    }

    void benchmark() {
        std::cout << "RAM benchmark:" << std::endl;
        std::cout << "--------------------------------------" << std::endl;

        std::string points;
        volatile int sum = 0;
        volatile int trash = 0;

        // Random
        //----------------
        std::cout << "Random: " << std::flush;
        // Random read
        sum = 0;

        auto s = std::chrono::high_resolution_clock::now();

        for (unsigned i = 0; i < bo->iterations; ++i) {
            for (unsigned j = 0; j < bo->vectorSize; ++j) {
                sum += bo->vecA[(size_t)bo->vecB[j]];
            }
        }

        auto e = std::chrono::high_resolution_clock::now();
        auto t = std::chrono::duration_cast<std::chrono::microseconds>(e - s);

        compute_points(bo->vectorSize, bo->iterations, t.count(), &points);
        std::cout << "Read: " << points << std::flush;

        // Random write
        volatile int magicValue = 0;

        s = std::chrono::high_resolution_clock::now();

        for (unsigned i = 0; i < bo->iterations; ++i) {
            for (unsigned j = 0; j < bo->vectorSize; ++j) {
                bo->vecA[(size_t)bo->vecB[j]] = magicValue;
            }
        }

        e = std::chrono::high_resolution_clock::now();
        t = std::chrono::duration_cast<std::chrono::microseconds>(e - s);

        compute_points(bo->vectorSize, bo->iterations, t.count(), &points);
        std::cout << "\t Write: " << points << std::flush;
        std::cout << std::endl;

        // Sequential
        //----------------
        std::cout << "Sequential: " << std::flush;
        // Sequential read
        sum = 0;

        s = std::chrono::high_resolution_clock::now();

        for (unsigned i = 0; i < bo->iterations; ++i) {
            for (unsigned j = 0; j < bo->vectorSize; ++j) {
                trash = bo->vecB[j];
                sum += bo->vecA[j];
            }
        }

        e = std::chrono::high_resolution_clock::now();
        t = std::chrono::duration_cast<std::chrono::microseconds>(e - s);

        compute_points(bo->vectorSize, bo->iterations, t.count(), &points);
        std::cout << "Read: " << points << std::flush;

        // Sequential write
        magicValue = 0;

        s = std::chrono::high_resolution_clock::now();

        for (unsigned i = 0; i < bo->iterations; ++i) {
            for (unsigned j = 0; j < bo->vectorSize; ++j) {
                trash = bo->vecB[j];
                bo->vecA[j] = magicValue;
            }
        }

        e = std::chrono::high_resolution_clock::now();
        t = std::chrono::duration_cast<std::chrono::microseconds>(e - s);

        compute_points(bo->vectorSize, bo->iterations, t.count(), &points);
        std::cout << "\t Write: " << points << std::flush;
        std::cout << std::endl;

        std::cout << std::endl;
    }
};
