#include <iostream>
#include <windows.h>
#include <tlhelp32.h>
#include <string>
#include <filesystem>
#include <vector>

namespace fs = std::filesystem;

DWORD get_process_id(const std::wstring& process_name) {
    DWORD pid = 0;
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot != INVALID_HANDLE_VALUE) {
        PROCESSENTRY32W entry;
        entry.dwSize = sizeof(entry);
        if (Process32FirstW(snapshot, &entry)) {
            do {
                if (process_name == entry.szExeFile) {
                    pid = entry.th32ProcessID;
                    break;
                }
            } while (Process32NextW(snapshot, &entry));
        }
        CloseHandle(snapshot);
    }
    return pid;
}

bool inject_dll(DWORD pid, const std::wstring& dll_path) {
    HANDLE process = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    if (!process) {
        std::wcerr << L"Failed to open process. Error: " << GetLastError() << std::endl;
        return false;
    }

    size_t size = (dll_path.length() + 1) * sizeof(wchar_t);
    void* loc = VirtualAllocEx(process, nullptr, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!loc) {
        std::wcerr << L"Failed to allocate memory in target process. Error: " << GetLastError() << std::endl;
        CloseHandle(process);
        return false;
    }

    if (!WriteProcessMemory(process, loc, dll_path.c_str(), size, nullptr)) {
        std::wcerr << L"Failed to write memory in target process. Error: " << GetLastError() << std::endl;
        VirtualFreeEx(process, loc, 0, MEM_RELEASE);
        CloseHandle(process);
        return false;
    }

    HANDLE thread = CreateRemoteThread(process, nullptr, 0, (LPTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandleA("kernel32.dll"), "LoadLibraryW"), loc, 0, nullptr);
    if (!thread) {
        std::wcerr << L"Failed to create remote thread. Error: " << GetLastError() << std::endl;
        VirtualFreeEx(process, loc, 0, MEM_RELEASE);
        CloseHandle(process);
        return false;
    }

    std::wcout << L"Successfully injected DLL into process " << pid << std::endl;

    CloseHandle(thread);
    CloseHandle(process);
    return true;
}

int main() {
    std::wcout << L"--- R6 External Launcher (User-Mode) ---" << std::endl;

    // Use wide string for potential non-ASCII characters in paths
    std::wstring dll_name = L"rainbowsix-external.dll";
    fs::path dll_path = fs::current_path() / dll_name;

    if (!fs::exists(dll_path)) {
        std::wcerr << L"Error: " << dll_name << L" not found in the current directory." << std::endl;
        system("pause");
        return 1;
    }

    std::wcout << L"Waiting for dwm.exe..." << std::endl;
    DWORD pid = 0;
    while (pid == 0) {
        pid = get_process_id(L"dwm.exe");
        Sleep(500);
    }

    std::wcout << L"Found dwm.exe (PID: " << pid << L"). Injecting..." << std::endl;

    if (inject_dll(pid, dll_path.wstring())) {
        std::wcout << L"Cheat successfully launched in User-Mode." << std::endl;
    } else {
        std::wcerr << L"Injection failed. Make sure to run the launcher as Administrator." << std::endl;
    }

    system("pause");
    return 0;
}
