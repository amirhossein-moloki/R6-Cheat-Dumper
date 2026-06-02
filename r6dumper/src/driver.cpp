#include "driver.hpp"

bool KernelInterface::Initialize() {
    m_hDevice = CreateFileW(L"\\\\.\\Global\\MemDrv", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    return m_hDevice != INVALID_HANDLE_VALUE;
}

void KernelInterface::Shutdown() {
    if (m_hDevice != INVALID_HANDLE_VALUE) {
        CloseHandle(m_hDevice);
        m_hDevice = INVALID_HANDLE_VALUE;
    }
}

bool KernelInterface::ReadMemory(HANDLE pid, UINT64 address, void* buffer, UINT64 size) {
    MEMORY_REQUEST request;
    request.ProcessId = pid;
    request.Address = address;
    request.Buffer = buffer;
    request.Size = size;

    return DeviceIoControl(m_hDevice, IOCTL_MEMORY_READ, &request, sizeof(request), &request, sizeof(request), NULL, NULL);
}

bool KernelInterface::WriteMemory(HANDLE pid, UINT64 address, void* buffer, UINT64 size) {
    MEMORY_REQUEST request;
    request.ProcessId = pid;
    request.Address = address;
    request.Buffer = buffer;
    request.Size = size;

    return DeviceIoControl(m_hDevice, IOCTL_MEMORY_WRITE, &request, sizeof(request), &request, sizeof(request), NULL, NULL);
}

UINT64 KernelInterface::GetModuleBase(HANDLE pid, const wchar_t* moduleName) {
    MODULE_REQUEST request;
    request.ProcessId = pid;
    if (moduleName) {
        wcscpy_s(request.ModuleName, moduleName);
    } else {
        request.ModuleName[0] = L'\0';
    }

    if (DeviceIoControl(m_hDevice, IOCTL_MODULE_BASE, &request, sizeof(request), &request, sizeof(request), NULL, NULL)) {
        return request.BaseAddress;
    }
    return 0;
}

HANDLE KernelInterface::GetProcessId(const wchar_t* processName) {
    PID_REQUEST request;
    wcscpy_s(request.ProcessName, processName);

    if (DeviceIoControl(m_hDevice, IOCTL_PROCESS_ID, &request, sizeof(request), &request, sizeof(request), NULL, NULL)) {
        return request.ProcessId;
    }
    return NULL;
}

// Legacy wrappers
namespace driver {
    KernelInterface* interface = new KernelInterface();

    bool initialize() {
        return interface->Initialize();
    }

    uint64_t open_process(uint32_t pid) {
        // In the new system, handle is just PID
        return (uint64_t)pid;
    }

    bool read_memory(uint64_t handle, uint64_t address, void* buffer, uint32_t size) {
        return interface->ReadMemory((HANDLE)handle, address, buffer, (UINT64)size);
    }

    bool write_memory(uint64_t handle, uint64_t address, const void* buffer, uint32_t size) {
        return interface->WriteMemory((HANDLE)handle, address, (void*)buffer, (UINT64)size);
    }

    uint64_t get_module_base(uint64_t handle, const wchar_t* dllname) {
        return interface->GetModuleBase((HANDLE)handle, dllname);
    }
}