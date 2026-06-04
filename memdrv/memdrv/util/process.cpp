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
        if (!processName || !pid) return STATUS_INVALID_PARAMETER;

        ULONG size = 0;
        NTSTATUS status = ZwQuerySystemInformation(SystemProcessInformation, nullptr, 0, &size);

        // Security: Implement a retry loop to handle race conditions where process list grows
        PVOID buffer = nullptr;
        while (status == STATUS_INFO_LENGTH_MISMATCH || size > 0) {
            if (buffer) {
                ExFreePoolWithTag(buffer, 'PROC');
            }

            // Add a small buffer to avoid immediate mismatch if a few processes start
            size += 4096;

            buffer = ExAllocatePool2(POOL_FLAG_PAGED, size, 'PROC');
            if (!buffer) return STATUS_INSUFFICIENT_RESOURCES;

            status = ZwQuerySystemInformation(SystemProcessInformation, buffer, size, &size);
            if (NT_SUCCESS(status)) {
                break;
            }
        }

        if (NT_SUCCESS(status) && buffer) {
            auto currentEntry = (PSYSTEM_PROCESS_INFORMATION)buffer;
            status = STATUS_NOT_FOUND; // Default if not found in loop

            while (true) {
                // Security: ImageName.Buffer is a user-mode pointer in some contexts,
                // but ZwQuerySystemInformation returns it as a kernel buffer here.
                if (currentEntry->ImageName.Buffer && currentEntry->ImageName.Length > 0) {
                    if (_wcsicmp(currentEntry->ImageName.Buffer, processName) == 0) {
                        *pid = currentEntry->UniqueProcessId;
                        status = STATUS_SUCCESS;
                        break;
                    }
                }
                if (currentEntry->NextEntryOffset == 0) {
                    break;
                }
                currentEntry = (PSYSTEM_PROCESS_INFORMATION)((PUCHAR)currentEntry + currentEntry->NextEntryOffset);
            }
        }

        if (buffer) {
            ExFreePoolWithTag(buffer, 'PROC');
        }
        return status;
    }
}
