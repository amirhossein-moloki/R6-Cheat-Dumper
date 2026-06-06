#include "driver.hpp"
#include "../core/logger.hpp"

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

bool KernelInterface::WriteMemory(HANDLE pid, UINT64 address, const void* buffer, UINT64 size) {
    MEMORY_REQUEST request;
    request.ProcessId = pid;
    request.Address = address;
    request.Buffer = const_cast<void*>(buffer);
    request.Size = size;
    return DeviceIoControl(m_hDevice, IOCTL_MEMORY_WRITE, &request, sizeof(request), &request, sizeof(request), NULL, NULL);
}

UINT64 KernelInterface::GetModuleBase(HANDLE pid, const wchar_t* moduleName) {
    MODULE_REQUEST request;
    request.ProcessId = pid;
    if (moduleName) wcscpy_s(request.ModuleName, moduleName);
    else request.ModuleName[0] = L'\0';

    if (DeviceIoControl(m_hDevice, IOCTL_MODULE_BASE, &request, sizeof(request), &request, sizeof(request), NULL, NULL))
        return request.BaseAddress;
    return 0;
}

HANDLE KernelInterface::GetProcessId(const wchar_t* processName) {
    PID_REQUEST request;
    wcscpy_s(request.ProcessName, processName);
    if (DeviceIoControl(m_hDevice, IOCTL_PROCESS_ID, &request, sizeof(request), &request, sizeof(request), NULL, NULL))
        return request.ProcessId;
    return NULL;
}

namespace driver {
	bool g_user_mode = false;
	bool g_has_write_access = false;
    KernelInterface* g_interface = new KernelInterface();

	void set_user_mode(bool user_mode) {
		g_user_mode = user_mode;
	}

	bool initialize() {
		if (g_user_mode) return true;

		if (g_interface->Initialize()) {
			LOG_INFO("Driver connection established.");
			g_has_write_access = true;
			return true;
		}

		LOG_WARN("Driver not found. Falling back to User-mode.");
		g_user_mode = true;
		g_has_write_access = false;
		return true;
	}

	uint64_t open_process(uint32_t pid) {
		if (g_user_mode) {
			HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_VM_OPERATION, FALSE, pid);
			if (hProcess != NULL) {
				g_has_write_access = true;
			}
			else {
				hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
				if (hProcess == NULL) return 0;
				g_has_write_access = false;
			}
			return reinterpret_cast<uint64_t>(hProcess);
		}
		return static_cast<uint64_t>(pid);
	}

	bool read_memory(uint64_t handle, uintptr_t address, uint8_t* buffer, uint32_t size) {
		if (g_user_mode) {
			SIZE_T bytes_read;
			return ReadProcessMemory(reinterpret_cast<HANDLE>(handle), reinterpret_cast<LPCVOID>(address), buffer, size, &bytes_read);
		}
		return g_interface->ReadMemory(reinterpret_cast<HANDLE>(handle), address, buffer, size);
	}

	bool write_memory(uint64_t handle, uintptr_t address, const uint8_t* buffer, uint32_t size) {
		if (g_user_mode) {
			SIZE_T bytes_written;
			return WriteProcessMemory(reinterpret_cast<HANDLE>(handle), reinterpret_cast<LPVOID>(address), const_cast<uint8_t*>(buffer), size, &bytes_written);
		}
		return g_interface->WriteMemory(reinterpret_cast<HANDLE>(handle), address, buffer, size);
	}

	uintptr_t get_module_base(uint64_t handle, const wchar_t* dllname) {
		if (g_user_mode) {
			uintptr_t base = 0;
			HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, GetProcessId(reinterpret_cast<HANDLE>(handle)));
			if (hSnapshot != INVALID_HANDLE_VALUE) {
				MODULEENTRY32W me;
				me.dwSize = sizeof(me);
				if (Module32FirstW(hSnapshot, &me)) {
					do {
						if (_wcsicmp(me.szModule, dllname) == 0) {
							base = reinterpret_cast<uintptr_t>(me.modBaseAddr);
							break;
						}
					} while (Module32NextW(hSnapshot, &me));
				}
				CloseHandle(hSnapshot);
			}
			return base;
		}
		return static_cast<uintptr_t>(g_interface->GetModuleBase(reinterpret_cast<HANDLE>(handle), dllname));
	}
}