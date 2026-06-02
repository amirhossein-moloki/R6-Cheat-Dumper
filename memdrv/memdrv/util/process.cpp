#include "process.hpp"

// Internal NT structures not in WDK headers
typedef struct _SYSTEM_PROCESS_INFORMATION {
    ULONG NextEntryOffset;
    ULONG NumberOfThreads;
    LARGE_INTEGER WorkingSetPrivateSize;
    ULONG HardFaultCount;
    ULONG NumberOfThreadsHighWatermark;
    ULONGLONG CycleTime;
    LARGE_INTEGER CreateTime;
    LARGE_INTEGER UserTime;
    LARGE_INTEGER KernelTime;
    UNICODE_STRING ImageName;
    KPRIORITY BasePriority;
    HANDLE UniqueProcessId;
    HANDLE InheritedFromUniqueProcessId;
    ULONG HandleCount;
    ULONG SessionId;
    ULONG_PTR UniqueProcessKey;
    SIZE_T PeakVirtualSize;
    SIZE_T VirtualSize;
    ULONG PageFaultCount;
    SIZE_T PeakWorkingSetSize;
    SIZE_T WorkingSetSize;
    SIZE_T QuotaPeakPagedPoolUsage;
    SIZE_T QuotaPagedPoolUsage;
    SIZE_T PeakNonPagedPoolUsage;
    SIZE_T QuotaNonPagedPoolUsage;
    SIZE_T PagefileUsage;
    SIZE_T PeakPagefileUsage;
    SIZE_T PrivatePageCount;
    LARGE_INTEGER ReadOperationCount;
    LARGE_INTEGER WriteOperationCount;
    LARGE_INTEGER OtherOperationCount;
    LARGE_INTEGER ReadTransferCount;
    LARGE_INTEGER WriteTransferCount;
    LARGE_INTEGER OtherTransferCount;
} SYSTEM_PROCESS_INFORMATION, * PSYSTEM_PROCESS_INFORMATION;

typedef enum _SYSTEM_INFORMATION_CLASS {
    SystemProcessInformation = 5
} SYSTEM_INFORMATION_CLASS;

extern "C" {
    NTSTATUS NTAPI ZwQuerySystemInformation(ULONG SystemInformationClass, PVOID SystemInformation, ULONG SystemInformationLength, PULONG ReturnLength);
}

namespace util {
    NTSTATUS GetProcessIdByName(PCWSTR processName, PHANDLE pid) {
        // Simple implementation using PsGetNextProcess (if available) or ZwQuerySystemInformation
        // For simplicity and compatibility, we'll use ZwQuerySystemInformation with SystemProcessInformation

        ULONG size = 0;
        ZwQuerySystemInformation(SystemProcessInformation, nullptr, 0, &size);

        if (size == 0) return STATUS_UNSUCCESSFUL;

        // ExAllocatePool2 is available in latest WDK
        PVOID buffer = ExAllocatePool2(POOL_FLAG_PAGED, size, 'PROC');
        if (!buffer) return STATUS_INSUFFICIENT_RESOURCES;

        NTSTATUS status = ZwQuerySystemInformation(SystemProcessInformation, buffer, size, &size);
        if (NT_SUCCESS(status)) {
            auto currentEntry = (PSYSTEM_PROCESS_INFORMATION)buffer;
            while (true) {
                if (currentEntry->ImageName.Buffer && _wcsicmp(currentEntry->ImageName.Buffer, processName) == 0) {
                    *pid = currentEntry->UniqueProcessId;
                    status = STATUS_SUCCESS;
                    break;
                }
                if (currentEntry->NextEntryOffset == 0) {
                    status = STATUS_NOT_FOUND;
                    break;
                }
                currentEntry = (PSYSTEM_PROCESS_INFORMATION)((PUCHAR)currentEntry + currentEntry->NextEntryOffset);
            }
        }

        ExFreePoolWithTag(buffer, 'PROC');
        return status;
    }
}
