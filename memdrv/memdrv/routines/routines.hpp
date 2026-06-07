#ifndef _ROUTINES_HPP_
#define _ROUTINES_HPP_

#include "../defs.hpp"

namespace routines {
    NTSTATUS ReadProcessMemoryKernel(HANDLE pid, ULONGLONG address, UCHAR* buffer, ULONGLONG size);
    NTSTATUS WriteProcessMemoryKernel(HANDLE pid, ULONGLONG address, UCHAR* buffer, ULONGLONG size);
    NTSTATUS GetModuleBaseAddress(HANDLE pid, PCWSTR moduleName, PULONGLONG baseAddress);
    NTSTATUS GetProcessIdByName(PCWSTR processName, PHANDLE pid);
    PEPROCESS GetProcessByPid(HANDLE pid);
}

#endif