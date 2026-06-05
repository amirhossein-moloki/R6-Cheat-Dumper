#ifndef _DEFS_HPP_
#define _DEFS_HPP_

#include <ntifs.h>
#include <ntddk.h>
#include <windef.h>

#define IOCTL_BASE 0x800

#define IOCTL_MEMORY_READ     CTL_CODE(FILE_DEVICE_UNKNOWN, IOCTL_BASE + 1, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_MEMORY_WRITE    CTL_CODE(FILE_DEVICE_UNKNOWN, IOCTL_BASE + 2, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_MODULE_BASE     CTL_CODE(FILE_DEVICE_UNKNOWN, IOCTL_BASE + 3, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_PROCESS_ID      CTL_CODE(FILE_DEVICE_UNKNOWN, IOCTL_BASE + 4, METHOD_BUFFERED, FILE_ANY_ACCESS)

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
    WCHAR    ModuleName[256]; // Security: Must be null-terminated by caller or driver
    UINT64   BaseAddress;
    NTSTATUS Status;
} MODULE_REQUEST, *PMODULE_REQUEST;

typedef struct _PID_REQUEST {
    WCHAR    ProcessName[256]; // Security: Must be null-terminated by caller or driver
    HANDLE   ProcessId;
    NTSTATUS Status;
} PID_REQUEST, *PPID_REQUEST;
#pragma pack(pop)

#endif
