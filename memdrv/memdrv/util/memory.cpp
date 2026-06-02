#include "memory.hpp"

// Internal NT structures not in WDK headers
typedef struct _LDR_DATA_TABLE_ENTRY {
    LIST_ENTRY InLoadOrderLinks;
    LIST_ENTRY InMemoryOrderLinks;
    LIST_ENTRY InInitializationOrderLinks;
    PVOID DllBase;
    PVOID EntryPoint;
    ULONG SizeOfImage;
    UNICODE_STRING FullDllName;
    UNICODE_STRING BaseDllName;
    ULONG Flags;
    USHORT LoadCount;
    USHORT TlsIndex;
    union {
        LIST_ENTRY HashLinks;
        struct {
            PVOID SectionPointer;
            ULONG CheckSum;
        };
    };
    union {
        ULONG TimeDateStamp;
        PVOID LoadedImports;
    };
} LDR_DATA_TABLE_ENTRY, * PLDR_DATA_TABLE_ENTRY;

typedef struct _PEB_LDR_DATA {
    ULONG Length;
    UCHAR Initialized;
    PVOID SsHandle;
    LIST_ENTRY InLoadOrderModuleList;
    LIST_ENTRY InMemoryOrderModuleList;
    LIST_ENTRY InInitializationOrderModuleList;
    PVOID EntryInProgress;
} PEB_LDR_DATA, * PPEB_LDR_DATA;

typedef struct _PEB {
    UCHAR InheritedAddressSpace;
    UCHAR ReadImageFileExecOptions;
    UCHAR BeingDebugged;
    union {
        UCHAR BitField;
        struct {
            UCHAR ImageUsesLargePages : 1;
            UCHAR IsProtectedProcess : 1;
            UCHAR IsImageDynamicallyRelocated : 1;
            UCHAR SkipPatchingUser32Forwarders : 1;
            UCHAR IsPackagedProcess : 1;
            UCHAR IsAppContainer : 1;
            UCHAR IsProtectedProcessLight : 1;
            UCHAR IsLongPathAwareProcess : 1;
        };
    };
    HANDLE Mutant;
    PVOID ImageBaseAddress;
    PPEB_LDR_DATA Ldr;
    // ... more fields can be added if needed
} PEB, * PPEB;

extern "C" {
    NTKERNELAPI PPEB NTAPI PsGetProcessPeb(PEPROCESS Process);
    NTKERNELAPI PVOID NTAPI PsGetProcessSectionBaseAddress(PEPROCESS Process);
}

namespace util {
    NTSTATUS GetProcessModuleBase(PEPROCESS process, PCWSTR moduleName, PUINT64 baseAddress) {
        if (!process) return STATUS_INVALID_PARAMETER;

        // Use PsGetProcessSectionBaseAddress for the main module
        if (moduleName == nullptr || moduleName[0] == L'\0') {
             // We can now use the declared function directly if available at link time,
             // but keeping dynamic resolution as a fallback or for safety if preferred.
             // Actually, since it was in defs.hpp before, it's likely linked.
             *baseAddress = (UINT64)PsGetProcessSectionBaseAddress(process);
             return *baseAddress ? STATUS_SUCCESS : STATUS_NOT_FOUND;
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
