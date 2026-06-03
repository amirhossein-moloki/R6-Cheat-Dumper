#include "driver.hpp"

#include <Windows.h>

namespace driver {
    HANDLE m_hDevice = INVALID_HANDLE_VALUE;

	bool is_driver_loaded() {
        if (m_hDevice != INVALID_HANDLE_VALUE) return true;

        m_hDevice = CreateFileW(L"\\\\.\\Global\\MemDrv", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
        return m_hDevice != INVALID_HANDLE_VALUE;
	}

	uint64_t open_process(uint32_t pid) {
        // In the new system, handle is just PID
		return (uint64_t)pid;
	}

	bool read_memory(uint64_t handle, uintptr_t address, uint8_t* buffer, uint32_t size) {
        if (!is_driver_loaded()) return false;

        MEMORY_REQUEST request;
        request.ProcessId = (HANDLE)handle;
        request.Address = address;
        request.Buffer = buffer;
        request.Size = size;

        if (DeviceIoControl(m_hDevice, IOCTL_MEMORY_READ, &request, sizeof(request), &request, sizeof(request), NULL, NULL)) {
            return request.Status == 0; // STATUS_SUCCESS
        }
        return false;
	}

	bool write_memory(uint64_t handle, uintptr_t address, const uint8_t* buffer, uint32_t size) {
        if (!is_driver_loaded()) return false;

        MEMORY_REQUEST request;
        request.ProcessId = (HANDLE)handle;
        request.Address = address;
        request.Buffer = (void*)buffer;
        request.Size = size;

        if (DeviceIoControl(m_hDevice, IOCTL_MEMORY_WRITE, &request, sizeof(request), &request, sizeof(request), NULL, NULL)) {
            return request.Status == 0; // STATUS_SUCCESS
        }
        return false;
	}

	bool protect_virtual_memory(uint64_t handle, uintptr_t address, uint32_t size, uint32_t protect, uint32_t* old_protect) {
        // Not implemented in memdrv
        (void)handle; (void)address; (void)size; (void)protect; (void)old_protect;
		return false;
	}

	uintptr_t get_module_base(uint64_t handle, const wchar_t* dllname) {
        if (!is_driver_loaded()) return 0;

        MODULE_REQUEST request;
        request.ProcessId = (HANDLE)handle;
        if (dllname) {
            wcscpy_s(request.ModuleName, 256, dllname);
        } else {
            request.ModuleName[0] = L'\0';
        }

        if (DeviceIoControl(m_hDevice, IOCTL_MODULE_BASE, &request, sizeof(request), &request, sizeof(request), NULL, NULL)) {
            if (request.Status == 0)
                return (uintptr_t)request.BaseAddress;
        }
        return 0;
	}
}
