#include "driver.hpp"

bool KernelInterface::Initialize() {
    m_hDevice = CreateFileW(L"\\\\.\\Global\\MemDrv", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    if (m_hDevice == INVALID_HANDLE_VALUE) {
        return false;
    }
    return true;
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
    KernelInterface* g_interface = new KernelInterface();
    bool g_user_mode = false;

    void set_user_mode(bool user_mode) {
        g_user_mode = user_mode;
    }

    bool initialize() {
        if (g_user_mode) {
            std::cout << "[*] User-mode selected." << std::endl;
            return true;
        }

        if (g_interface->Initialize()) {
            std::cout << "[+] Driver connection established." << std::endl;
            return true;
        }

        std::cout << "[!] Driver not found. Falling back to User-mode (ReadProcessMemory)." << std::endl;
        g_user_mode = true;
        return true;
    }

    uint64_t open_process(uint32_t pid) {
        if (g_user_mode) {
            std::cout << "[*] Opening process " << pid << " with limited rights..." << std::endl;
            HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
            if (hProcess == NULL) {
                DWORD error = GetLastError();
                std::cout << "[-] OpenProcess failed. Error code: " << error << " (0x" << std::hex << error << std::dec << ")" << std::endl;
                return 0;
            }
            std::cout << "[+] Handle obtained: 0x" << std::hex << (uintptr_t)hProcess << std::dec << std::endl;
            return (uint64_t)hProcess;
        }
        // In the kernel system, handle is just PID
        return (uint64_t)pid;
    }

    bool read_memory(uint64_t handle, uint64_t address, void* buffer, uint32_t size) {
        if (g_user_mode) {
            SIZE_T bytesRead;
            if (ReadProcessMemory((HANDLE)handle, (LPCVOID)address, buffer, size, &bytesRead)) {
                return true;
            }
            return false;
        }
        return g_interface->ReadMemory((HANDLE)handle, address, buffer, (UINT64)size);
    }

    bool write_memory(uint64_t handle, uint64_t address, const void* buffer, uint32_t size) {
        if (g_user_mode) {
            SIZE_T bytesWritten;
            return WriteProcessMemory((HANDLE)handle, (LPVOID)address, buffer, size, &bytesWritten);
        }
        return g_interface->WriteMemory((HANDLE)handle, address, (void*)buffer, (UINT64)size);
    }

    uint64_t get_module_base(uint64_t handle, const wchar_t* dllname) {
        if (g_user_mode) {
            uint64_t base = 0;
            HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, GetProcessId((HANDLE)handle));
            if (hSnapshot != INVALID_HANDLE_VALUE) {
                MODULEENTRY32W me;
                me.dwSize = sizeof(me);
                if (Module32FirstW(hSnapshot, &me)) {
                    do {
                        if (_wcsicmp(me.szModule, dllname) == 0) {
                            base = (uint64_t)me.modBaseAddr;
                            break;
                        }
                    } while (Module32NextW(hSnapshot, &me));
                }
                CloseHandle(hSnapshot);
            }
            return base;
        }
        return g_interface->GetModuleBase((HANDLE)handle, dllname);
    }
}