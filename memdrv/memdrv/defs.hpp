#ifndef _DEFS_HPP_
#define _DEFS_HPP_

#include <ntifs.h>
#include <ntddk.h>
#include <windef.h>

#define IOCTL_BASE 0x800

#define IOCTL_MEMORY_READ     CTL_CODE(FILE_DEVICE_UNKNOWN, IOCTL_BASE + 1, METHOD_NEITHER, FILE_ANY_ACCESS)
#define IOCTL_MEMORY_WRITE    CTL_CODE(FILE_DEVICE_UNKNOWN, IOCTL_BASE + 2, METHOD_NEITHER, FILE_ANY_ACCESS)
#define IOCTL_MODULE_BASE     CTL_CODE(FILE_DEVICE_UNKNOWN, IOCTL_BASE + 3, METHOD_NEITHER, FILE_ANY_ACCESS)
#define IOCTL_PROCESS_ID      CTL_CODE(FILE_DEVICE_UNKNOWN, IOCTL_BASE + 4, METHOD_NEITHER, FILE_ANY_ACCESS)

// Shared between kernel and user mode
#pragma pack(push, 1)
typedef struct _MEMORY_REQUEST {
    HANDLE   ProcessId;
    UINT64   Address;
    UINT64   Size;
    UINT8*   Buffer;
    NTSTATUS Status;
} MEMORY_REQUEST, *PMEMORY_REQUEST;

typedef struct _MODULE_REQUEST {
    HANDLE   ProcessId;
    WCHAR    ModuleName[256];
    UINT64   BaseAddress;
    NTSTATUS Status;
} MODULE_REQUEST, *PMODULE_REQUEST;

typedef struct _PID_REQUEST {
    WCHAR    ProcessName[256];
    HANDLE   ProcessId;
    NTSTATUS Status;
} PID_REQUEST, *PPID_REQUEST;
#pragma pack(pop)

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
    SIZE_T QuotaPeakNonPagedPoolUsage;
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

typedef enum _SYSTEM_INFORMATION_CLASS {
    SystemProcessInformation = 5
} SYSTEM_INFORMATION_CLASS;

extern "C" {
    NTKERNELAPI PPEB NTAPI PsGetProcessPeb(PEPROCESS Process);
    NTKERNELAPI PVOID NTAPI PsGetProcessSectionBaseAddress(PEPROCESS Process);
    NTSTATUS NTAPI ZwQuerySystemInformation(ULONG SystemInformationClass, PVOID SystemInformation, ULONG SystemInformationLength, PULONG ReturnLength);
}

#endif