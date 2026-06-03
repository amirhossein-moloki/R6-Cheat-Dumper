#ifndef _DRIVER_DRIVER_HPP_
#define _DRIVER_DRIVER_HPP_

#include <Windows.h>
#include <iostream>

#define IOCTL_BASE 0x800

#define IOCTL_MEMORY_READ     CTL_CODE(FILE_DEVICE_UNKNOWN, IOCTL_BASE + 1, METHOD_NEITHER, FILE_ANY_ACCESS)
#define IOCTL_MEMORY_WRITE    CTL_CODE(FILE_DEVICE_UNKNOWN, IOCTL_BASE + 2, METHOD_NEITHER, FILE_ANY_ACCESS)
#define IOCTL_MODULE_BASE     CTL_CODE(FILE_DEVICE_UNKNOWN, IOCTL_BASE + 3, METHOD_NEITHER, FILE_ANY_ACCESS)
#define IOCTL_PROCESS_ID      CTL_CODE(FILE_DEVICE_UNKNOWN, IOCTL_BASE + 4, METHOD_NEITHER, FILE_ANY_ACCESS)

#pragma pack(push, 1)
typedef struct _MEMORY_REQUEST {
    HANDLE   ProcessId;
    UINT64   Address;
    UINT64   Size;
    UINT8*   Buffer;
    NTSTATUS Status;
} MEMORY_REQUEST;

typedef struct _MODULE_REQUEST {
    HANDLE   ProcessId;
    WCHAR    ModuleName[256];
    UINT64   BaseAddress;
    NTSTATUS Status;
} MODULE_REQUEST;

typedef struct _PID_REQUEST {
    WCHAR    ProcessName[256];
    HANDLE   ProcessId;
    NTSTATUS Status;
} PID_REQUEST;
#pragma pack(pop)

namespace driver {
	extern bool is_driver_loaded();
	
	extern uint64_t open_process(uint32_t pid);

	extern bool read_memory(uint64_t handle, uintptr_t address, uint8_t* buffer, uint32_t size);
	extern bool write_memory(uint64_t handle, uintptr_t address, const uint8_t* buffer, uint32_t size);

	// Note: memdrv does not currently support ProtectVirtualMemory via IOCTL.
	// This function will remain for API compatibility but will return false.
	extern bool protect_virtual_memory(uint64_t handle, uintptr_t address, uint32_t size, uint32_t protect, uint32_t* old_protect);

	extern uintptr_t get_module_base(uint64_t handle, const wchar_t* dllname);
}
	
#endif
