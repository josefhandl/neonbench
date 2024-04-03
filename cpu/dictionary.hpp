// https://wiki.osdev.org/CPUID

// Vendor strings from CPUs.
#define CPUID_VENDOR_AMD           "AuthenticAMD"
#define CPUID_VENDOR_AMD_OLD       "AMDisbetter!" // Early engineering samples of AMD K5 processor
#define CPUID_VENDOR_INTEL         "GenuineIntel"
#define CPUID_VENDOR_VIA           "VIA VIA VIA "
#define CPUID_VENDOR_TRANSMETA     "GenuineTMx86"
#define CPUID_VENDOR_TRANSMETA_OLD "TransmetaCPU"
#define CPUID_VENDOR_CYRIX         "CyrixInstead"
#define CPUID_VENDOR_CENTAUR       "CentaurHauls"
#define CPUID_VENDOR_NEXGEN        "NexGenDriven"
#define CPUID_VENDOR_UMC           "UMC UMC UMC "
#define CPUID_VENDOR_SIS           "SiS SiS SiS "
#define CPUID_VENDOR_NSC           "Geode by NSC"
#define CPUID_VENDOR_RISE          "RiseRiseRise"
#define CPUID_VENDOR_VORTEX        "Vortex86 SoC"
#define CPUID_VENDOR_AO486         "MiSTer AO486"
#define CPUID_VENDOR_AO486_OLD     "GenuineAO486"
#define CPUID_VENDOR_ZHAOXIN       "  Shanghai  "
#define CPUID_VENDOR_HYGON         "HygonGenuine"
#define CPUID_VENDOR_ELBRUS        "E2K MACHINE "

// Vendor strings from hypervisors.
#define CPUID_VENDOR_QEMU          "TCGTCGTCGTCG"
#define CPUID_VENDOR_KVM           " KVMKVMKVM  "
#define CPUID_VENDOR_KVM_ALT       "KVMKVMKVM\0\0\0"
#define CPUID_VENDOR_VMWARE        "VMwareVMware"
#define CPUID_VENDOR_VIRTUALBOX    "VBoxVBoxVBox"
#define CPUID_VENDOR_XEN           "XenVMMXenVMM"
#define CPUID_VENDOR_HYPERV        "Microsoft Hv"
#define CPUID_VENDOR_PARALLELS     " prl hyperv "
#define CPUID_VENDOR_PARALLELS_ALT " lrpepyh vr " // Sometimes Parallels incorrectly encodes "prl hyperv" as "lrpepyh vr" due to an endianness mismatch.
#define CPUID_VENDOR_BHYVE         "bhyve bhyve "
#define CPUID_VENDOR_QNX           " QNXQVMBSQG "

#define HYPERVISOR_VENDOR_QEMU        "Qemu"
#define HYPERVISOR_VENDOR_KVM         "KVM"
#define HYPERVISOR_VENDOR_VMWARE      "VMware"
#define HYPERVISOR_VENDOR_VIRTUALBOX  "VirtualBox"
#define HYPERVISOR_VENDOR_XEN         "Xen"
#define HYPERVISOR_VENDOR_HYPERV      "Microsoft Hyper-V or Windows Virtual PC"
#define HYPERVISOR_VENDOR_PARALLELS   "Parallels"
#define HYPERVISOR_VENDOR_BHYVE       "bhyve"
#define HYPERVISOR_VENDOR_QNX         "QNX"


#define CPUID_MAGIC_QUERY_BASIC_INFO    0x0000000
#define CPUID_MAGIC_QUERY_EXTENDED_INFO 0x8000000
#define CPUID_MAGIC_QUERY_INFO_CACHE    0x0000004
#define CPUID_MAGIC_QUERY_VENDOR_ID    0x40000000
