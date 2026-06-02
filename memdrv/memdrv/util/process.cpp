#include "process.hpp"

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