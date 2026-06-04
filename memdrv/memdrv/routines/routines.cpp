#include "routines.hpp"
#include "../util/process.hpp"
#include "../util/memory.hpp"

namespace routines {

    PEPROCESS GetProcessByPid(HANDLE pid) {
        PEPROCESS process = nullptr;
        if (NT_SUCCESS(PsLookupProcessByProcessId(pid, &process))) {
            return process;
        }
        return nullptr;
    }

    NTSTATUS ReadProcessMemoryKernel(HANDLE pid, UINT64 address, UINT8* buffer, UINT64 size) {
        if (!pid || !address || !buffer || !size) return STATUS_INVALID_PARAMETER;

        PEPROCESS process = GetProcessByPid(pid);
        if (!process) return STATUS_NOT_FOUND;

        SIZE_T bytesCopied = 0;
        // Use MmCopyVirtualMemory which is a common way to read/write memory between processes in kernel
        // Note: It's technically internal but widely used.
        typedef NTSTATUS(NTAPI* PMMCOPYVIRTUALMEMORY)(PEPROCESS, PVOID, PEPROCESS, PVOID, SIZE_T, KPROCESSOR_MODE, PSIZE_T);
        static PMMCOPYVIRTUALMEMORY MmCopyVirtualMemoryPtr = nullptr;

        if (!MmCopyVirtualMemoryPtr) {
            UNICODE_STRING routineName = RTL_CONSTANT_STRING(L"MmCopyVirtualMemory");
            MmCopyVirtualMemoryPtr = (PMMCOPYVIRTUALMEMORY)MmGetSystemRoutineAddress(&routineName);
        }

        NTSTATUS status = STATUS_NOT_IMPLEMENTED;
        if (MmCopyVirtualMemoryPtr) {
            // Note: buffer is already probed in IoControl
            status = MmCopyVirtualMemoryPtr(process, (PVOID)address, PsGetCurrentProcess(), buffer, (SIZE_T)size, KernelMode, &bytesCopied);
        }

        ObDereferenceObject(process);
        return status;
    }

    NTSTATUS WriteProcessMemoryKernel(HANDLE pid, UINT64 address, UINT8* buffer, UINT64 size) {
        if (!pid || !address || !buffer || !size) return STATUS_INVALID_PARAMETER;

        PEPROCESS process = GetProcessByPid(pid);
        if (!process) return STATUS_NOT_FOUND;

        SIZE_T bytesCopied = 0;
        typedef NTSTATUS(NTAPI* PMMCOPYVIRTUALMEMORY)(PEPROCESS, PVOID, PEPROCESS, PVOID, SIZE_T, KPROCESSOR_MODE, PSIZE_T);
        static PMMCOPYVIRTUALMEMORY MmCopyVirtualMemoryPtr = nullptr;

        if (!MmCopyVirtualMemoryPtr) {
            UNICODE_STRING routineName = RTL_CONSTANT_STRING(L"MmCopyVirtualMemory");
            MmCopyVirtualMemoryPtr = (PMMCOPYVIRTUALMEMORY)MmGetSystemRoutineAddress(&routineName);
        }

        NTSTATUS status = STATUS_NOT_IMPLEMENTED;
        if (MmCopyVirtualMemoryPtr) {
            // Note: buffer is already probed in IoControl
            status = MmCopyVirtualMemoryPtr(PsGetCurrentProcess(), buffer, process, (PVOID)address, (SIZE_T)size, KernelMode, &bytesCopied);
        }

        ObDereferenceObject(process);
        return status;
    }

    NTSTATUS GetModuleBaseAddress(HANDLE pid, PCWSTR moduleName, PUINT64 baseAddress) {
        PEPROCESS process = GetProcessByPid(pid);
        if (!process) return STATUS_NOT_FOUND;

        NTSTATUS status = util::GetProcessModuleBase(process, moduleName, baseAddress);

        ObDereferenceObject(process);
        return status;
    }

    NTSTATUS GetProcessIdByName(PCWSTR processName, PHANDLE pid) {
        return util::GetProcessIdByName(processName, pid);
    }
}