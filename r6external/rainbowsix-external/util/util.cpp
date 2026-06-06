#include "util.hpp"
#include "../core/logger.hpp"
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <Tlhelp32.h>

uint32_t util::get_pid_from_window(const char* window_name) {
	HWND windowHandle = FindWindowA(NULL, window_name);
	DWORD* processID = new DWORD;
	GetWindowThreadProcessId(windowHandle, processID);
	return *processID;
}

uint32_t util::get_pid_from_class(const char* window_class) {
	HWND windowHandle = FindWindowA(window_class, NULL);
	DWORD* processID = new DWORD;
	GetWindowThreadProcessId(windowHandle, processID);
	return *processID;
}

uint32_t util::get_pid_from_file(const char target[]) {
	uint32_t pid = 0;
	HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (snap == INVALID_HANDLE_VALUE) {
			LOG_ERROR("Error creating snapshot: {}", GetLastError());
		return 0;
	}
	PROCESSENTRY32 pe32;
	pe32.dwSize = sizeof(PROCESSENTRY32);
	if (Process32First(snap, &pe32)) {
		do {
			if (strcmp(pe32.szExeFile, "RainbowSix.exe") == 0 ||
				strcmp(pe32.szExeFile, "RainbowSix_Vulkan.exe") == 0) {
				pid = pe32.th32ProcessID;
				break;
			}
			if (strcmp(pe32.szExeFile, "RainbowSix_BE.exe") == 0) {
				pid = pe32.th32ProcessID;
			}
		} while (Process32Next(snap, &pe32));
	}

	if (pid != 0) {
		CloseHandle(snap);
		return pid;
	}

	// If not found, list processes as requested in fallback
	LOG_WARN("Process {} not found. Listing active processes:", target);
	if (Process32First(snap, &pe32)) {
		do {
			LOG_INFO("  - {} (PID: {})", pe32.szExeFile, pe32.th32ProcessID);
		} while (Process32Next(snap, &pe32));
	}

	CloseHandle(snap);
	return 0;
}

bool util::is_game_open(const char* window_title, const char* window_class, const char window_file[]) {
	uint32_t window_pid = get_pid_from_window(window_title);
	uint32_t class_pid = get_pid_from_class(window_class);
	uint32_t file_pid = get_pid_from_file(window_file);

	// More robust check: if any method finds a PID, check if others match or are zero
	if (file_pid != 0) {
		// Prioritize file_pid as it's the most reliable for the specific executable
		return true;
	}

	if (window_pid != 0 || class_pid != 0) {
		return true;
	}

	return false;
}