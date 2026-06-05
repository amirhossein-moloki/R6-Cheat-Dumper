#include "driver.hpp"

#include <Windows.h>
#include <TlHelp32.h>

namespace driver {
	bool g_user_mode = false;
	bool g_has_write_access = false;

	void set_user_mode(bool user_mode) {
		g_user_mode = user_mode;
	}

	constexpr auto SYSCALL_UNIQUE = 0x133;

	using fnNtConvertBetweenAuxiliaryCounterAndPerformanceCounter = void* (NTAPI*)(void*, void*, void*, void*);

	struct open_handle_data {
		uint32_t pid;
		uint64_t handle;
	};

	struct read_write_data {
		uint64_t handle;
		uint64_t address;
		uint64_t buffer;

		uint32_t size;
	};

	struct virtual_protect_data {
		uint64_t handle;
		uint64_t address;
		uint32_t size;
		uint32_t protection;
		uint32_t old_protection;
	};

	struct get_module_base_data {
		uint64_t handle;
		const wchar_t* dllname;
		uint32_t dllname_length;

		uintptr_t module_base;
	};

	enum struct DriverCall {
		IsLoaded,
		OpenProcess,
		ReadProcessMemory,
		WriteProcessMemory,
		AllocateVirtualMemory,
		FreeVirtualMemory,
		ProtectVirtualMemory,
		GetModuleBase
	};

	struct syscall_data {
		uint32_t unique;
		uint32_t syscall;
		uint64_t arguments;
	};

	NTSTATUS execute_syscall(DriverCall syscall, void* args) {
		syscall_data data = {};

		data.unique = SYSCALL_UNIQUE;
		data.syscall = static_cast<std::uint32_t>(syscall);
		data.arguments = reinterpret_cast<uint64_t>(args);

		void* pdata = &data;

		static fnNtConvertBetweenAuxiliaryCounterAndPerformanceCounter oNtConvertBetweenAuxiliaryCounterAndPerformanceCounter = nullptr;

		if (oNtConvertBetweenAuxiliaryCounterAndPerformanceCounter == nullptr) {
			auto ntdll = GetModuleHandleA("ntdll.dll");

			if (ntdll == nullptr)
				ntdll = LoadLibraryA("ntdll.dll");

			if (ntdll == nullptr) return -1;

			oNtConvertBetweenAuxiliaryCounterAndPerformanceCounter = reinterpret_cast<
				fnNtConvertBetweenAuxiliaryCounterAndPerformanceCounter>(GetProcAddress(
					ntdll, "NtConvertBetweenAuxiliaryCounterAndPerformanceCounter"));
		}

		if (oNtConvertBetweenAuxiliaryCounterAndPerformanceCounter == nullptr) return -1;

		std::int64_t status = 0;
		oNtConvertBetweenAuxiliaryCounterAndPerformanceCounter(reinterpret_cast<void*>(1), &pdata, &status, nullptr);
		return static_cast<NTSTATUS>(status);
	}

	bool initialize() {
		if (g_user_mode) {
			std::cout << "[*] User-mode already active." << std::endl;
			return true;
		}

		if (is_driver_loaded()) {
			std::cout << "[+] Driver connection established." << std::endl;
			g_has_write_access = true;
			return true;
		}

		std::cout << "[!] Driver not found. Falling back to User-mode (ReadProcessMemory)." << std::endl;
		g_user_mode = true;
		g_has_write_access = false;
		return true;
	}

	bool is_driver_loaded() {
		if (g_user_mode) return true;

		bool status = false;
		NTSTATUS result = execute_syscall(DriverCall::IsLoaded, &status);

		return (result == 0 && status);
	}

	uint64_t open_process(uint32_t pid) {
		if (g_user_mode) {
			std::cout << "[*] Attempting to open process " << pid << " with full access..." << std::endl;
			HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_VM_OPERATION, FALSE, pid);
			if (hProcess != NULL) {
				std::cout << "[+] Full access handle obtained." << std::endl;
				g_has_write_access = true;
			}
			else {
				std::cout << "[!] Full access failed. Attempting read-only access..." << std::endl;
				hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
				if (hProcess == NULL) {
					DWORD error = GetLastError();
					std::cout << "[-] OpenProcess failed. Error code: " << error << " (0x" << std::hex << error << std::dec << ")" << std::endl;
					return 0;
				}
				std::cout << "[+] Read-only handle obtained." << std::endl;
				g_has_write_access = false;
			}
			std::cout << "[+] Handle obtained: 0x" << std::hex << reinterpret_cast<uint64_t>(hProcess) << std::dec << std::endl;
			return reinterpret_cast<uint64_t>(hProcess);
		}

		open_handle_data data = {};

		data.pid = pid;

		NTSTATUS status = execute_syscall(DriverCall::OpenProcess, &data);
		if (status != 0 || data.handle == 0) {
			std::cout << "[-] Driver failed to open process. Status: 0x" << std::hex << status << std::dec << std::endl;
			return 0;
		}

		return data.handle;
	}

	bool read_memory(uint64_t handle, uintptr_t address, uint8_t* buffer, uint32_t size) {
		if (g_user_mode) {
			SIZE_T bytes_read;
			if (ReadProcessMemory(reinterpret_cast<HANDLE>(handle), reinterpret_cast<LPCVOID>(address), buffer, size, &bytes_read)) {
				return true;
			}
			// Silent fail or diagnostic? Requested diagnostic prints for failures.
			// However, frequent reads might flood console. Let's stick to initial requirements.
			return false;
		}

		read_write_data data = {};

		data.handle = handle;
		data.address = address;
		data.buffer = reinterpret_cast<uint64_t>(buffer);
		data.size = size;

		return execute_syscall(DriverCall::ReadProcessMemory, &data) == 1;
	}

	bool write_memory(uint64_t handle, uintptr_t address, const uint8_t* buffer, uint32_t size) {
		if (g_user_mode) {
			SIZE_T bytes_written;
			if (WriteProcessMemory(reinterpret_cast<HANDLE>(handle), reinterpret_cast<LPVOID>(address), const_cast<uint8_t*>(buffer), size, &bytes_written)) {
				return true;
			}
			return false;
		}

		read_write_data data = {};

		data.handle = handle;
		data.address = address;
		data.buffer = reinterpret_cast<uint64_t>(buffer);
		data.size = size;

		return execute_syscall(DriverCall::WriteProcessMemory, &data) == 1;
	}

	bool protect_virtual_memory(uint64_t handle, uintptr_t address, uint32_t size, uint32_t protect, uint32_t* old_protect) {
		if (g_user_mode) {
			DWORD old_prot;
			bool status = VirtualProtectEx(reinterpret_cast<HANDLE>(handle), reinterpret_cast<LPVOID>(address), size, protect, &old_prot);
			if (old_protect) *old_protect = old_prot;
			return status;
		}

		virtual_protect_data data = {};

		data.handle = handle;
		data.address = address;
		data.size = size;
		data.protection = protect;

		*old_protect = protect;

		return execute_syscall(DriverCall::ProtectVirtualMemory, &data) == 1;
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

		get_module_base_data data = {};

		data.handle = handle;
		data.dllname = dllname;
		data.dllname_length = lstrlenW(dllname);

		(void)execute_syscall(DriverCall::GetModuleBase, &data);

		return data.module_base;
	}
}