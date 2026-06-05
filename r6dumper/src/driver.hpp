#ifndef _DRIVER_HPP_
#define _DRIVER_HPP_

#include <Windows.h>
#include <iostream>
#include <TlHelp32.h>

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
    void*    Buffer;
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

class KernelInterface {
private:
    HANDLE m_hDevice;
public:
    KernelInterface() : m_hDevice(INVALID_HANDLE_VALUE) {}
    bool Initialize();
    bool ReadMemory(HANDLE pid, UINT64 address, void* buffer, UINT64 size);
    bool WriteMemory(HANDLE pid, UINT64 address, void* buffer, UINT64 size);
    UINT64 GetModuleBase(HANDLE pid, const wchar_t* moduleName);
    HANDLE GetProcessId(const wchar_t* processName);
    void Shutdown();
};

// Legacy interface support if needed by other parts of the project
namespace driver {
    extern KernelInterface* g_interface;
    extern bool g_user_mode;

    void set_user_mode(bool user_mode);
    bool initialize();
    uint64_t open_process(uint32_t pid); // In new system, we use PID directly
    bool read_memory(uint64_t handle, uint64_t address, void* buffer, uint32_t size);
    bool write_memory(uint64_t handle, uint64_t address, const void* buffer, uint32_t size);
    uint64_t get_module_base(uint64_t handle, const wchar_t* dllname);
}

#endif