#include "memory.hpp"

namespace util {
    NTSTATUS GetProcessModuleBase(PEPROCESS process, PCWSTR moduleName, PUINT64 baseAddress) {
        if (!process) return STATUS_INVALID_PARAMETER;

        // Use PsGetProcessSectionBaseAddress for the main module
        if (moduleName == nullptr || moduleName[0] == L'\0') {
             typedef PVOID(NTAPI* PPSGETPROCESSSECTIONBASEADDRESS)(PEPROCESS);
             static PPSGETPROCESSSECTIONBASEADDRESS PsGetProcessSectionBaseAddressPtr = nullptr;

             if (!PsGetProcessSectionBaseAddressPtr) {
                 UNICODE_STRING routineName = RTL_CONSTANT_STRING(L"PsGetProcessSectionBaseAddress");
                 PsGetProcessSectionBaseAddressPtr = (PPSGETPROCESSSECTIONBASEADDRESS)MmGetSystemRoutineAddress(&routineName);
             }

             if (PsGetProcessSectionBaseAddressPtr) {
                 *baseAddress = (UINT64)PsGetProcessSectionBaseAddressPtr(process);
                 return STATUS_SUCCESS;
             }
             return STATUS_NOT_FOUND;
        }

        // For other modules, we would normally iterate PEB->Ldr
        // This requires attaching to the process
        KAPC_STATE apcState;
        KeStackAttachProcess(process, &apcState);

        // Standard way to get PEB
        PPEB peb = PsGetProcessPeb(process);
        NTSTATUS status = STATUS_NOT_FOUND;

        if (peb && peb->Ldr) {
            for (PLIST_ENTRY entry = peb->Ldr->InLoadOrderModuleList.Flink;
                 entry != &peb->Ldr->InLoadOrderModuleList;
                 entry = entry->Flink) {

                PLDR_DATA_TABLE_ENTRY module = CONTAINING_RECORD(entry, LDR_DATA_TABLE_ENTRY, InLoadOrderLinks);
                if (module->BaseDllName.Buffer && _wcsicmp(module->BaseDllName.Buffer, moduleName) == 0) {
                    *baseAddress = (UINT64)module->DllBase;
                    status = STATUS_SUCCESS;
                    break;
                }
            }
        }

        KeUnstackDetachProcess(&apcState);
        return status;
    }
}