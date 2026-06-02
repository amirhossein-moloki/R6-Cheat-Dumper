#ifndef _ROUTINES_HPP_
#define _ROUTINES_HPP_

#include "../defs.hpp"

namespace routines {
    NTSTATUS ReadProcessMemoryKernel(HANDLE pid, UINT64 address, UINT8* buffer, UINT64 size);
    NTSTATUS WriteProcessMemoryKernel(HANDLE pid, UINT64 address, UINT8* buffer, UINT64 size);
    NTSTATUS GetModuleBaseAddress(HANDLE pid, PCWSTR moduleName, PUINT64 baseAddress);
    NTSTATUS GetProcessIdByName(PCWSTR processName, PHANDLE pid);
    PEPROCESS GetProcessByPid(HANDLE pid);
}

#endif