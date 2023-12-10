
#include <iostream>

enum class NeonbenchSystemOS {
    Windows,
    MacOS,
    Linux
};

enum class NeonbenchSystemArch {
    X86,
    Arm
};

#ifdef _WIN32
    #define NEONBENCH_OS NeonbenchSystemOS::Windows
#elif __APPLE__
    #define NEONBENCH_OS NeonbenchSystemOS::MacOS
#elif __linux__
    #define NEONBENCH_OS NeonbenchSystemOS::Linux
#endif

#if defined(__i386__) || defined(__x86_64__)
    #define NEONBENCH_ARCH NeonbenchSystemArch::X86
#elif defined(__arm__) || defined(__aarch64__)
    #define NEONBENCH_ARCH NeonbenchSystemArch::Arm
#endif

class NeonbenchSystem {
    NeonbenchSystemOS os = NEONBENCH_OS;
    NeonbenchSystemArch arch = NEONBENCH_ARCH;

public:
    NeonbenchSystemOS getOS() const {
        return os;
    } 

    NeonbenchSystemArch getArch() const {
        return arch;
    }

    void printInfo() {
        std::cout << "System info:" << std::endl;
        std::cout << "--------------------------------------" << std::endl;
        std::cout << "Architecture: ";
        if (arch == NeonbenchSystemArch::X86)
            std::cout << "x86";
        else if (arch == NeonbenchSystemArch::Arm)
            std::cout << "arm";
        std::cout << std::endl;

        std::cout << "OS: ";
        if (os == NeonbenchSystemOS::Windows)
            std::cout << "Windows";
        else if (os == NeonbenchSystemOS::MacOS)
            std::cout << "macOS";
        else if (os == NeonbenchSystemOS::Linux)
            std::cout << "Linux";
        std::cout << std::endl;

        std::cout << std::endl;
    }
};
