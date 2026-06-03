#include "input.hpp"

#include <map>
#include <mutex>
#include <thread>
#include <algorithm>

#include <Windows.h>

namespace overlay::input {
    static std::mutex _key_mutex;

    static std::map<uint32_t, bool> _key_map;
    static std::map<uint32_t, bool> _key_read;

    static HHOOK _keyboard_hook = nullptr;
    static HHOOK _mouse_hook = nullptr;

    static std::thread _message_thread;

    LRESULT CALLBACK LowLevelMouseProc(
        _In_ int    nCode,
        _In_ WPARAM wParam,
        _In_ LPARAM lParam
    ) {
        return CallNextHookEx(_mouse_hook, nCode, wParam, lParam);
    }

    LRESULT CALLBACK LowLevelKeyboardProc(
        _In_ int    nCode,
        _In_ WPARAM wParam,
        _In_ LPARAM lParam
    ) {
        if (nCode >= 0) {
            const auto info = reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam);

            std::lock_guard<std::mutex> guard(_key_mutex);

            switch (wParam) {
            case WM_KEYDOWN:
            case WM_SYSKEYDOWN:
                _key_map[info->vkCode] = true;
                break;

            case WM_KEYUP:
            case WM_SYSKEYUP:
                _key_map[info->vkCode] = false;
                _key_read[info->vkCode] = false;
                break;

            default:
                break;
            }
        }

        return CallNextHookEx(_keyboard_hook, nCode, wParam, lParam);
    }

    void message_handler() {
        MSG msg;
        while (GetMessage(&msg, nullptr, 0, 0)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    void enable() {
        _message_thread = std::thread([] {
            _keyboard_hook = SetWindowsHookExA(WH_KEYBOARD_LL, &LowLevelKeyboardProc, GetModuleHandle(NULL), 0);

            if (_keyboard_hook == nullptr) {
                // MessageBoxA(nullptr, "error setting keyboard hook", "error", 0);
                return;
            }

            message_handler();
        });
        _message_thread.detach();
    }

    void disable() {
        if (_keyboard_hook) {
            UnhookWindowsHookEx(_keyboard_hook);
            _keyboard_hook = nullptr;
        }
    }

    bool key_down(uint32_t key) {
        std::lock_guard<std::mutex> guard(_key_mutex);
        return _key_map[key];
    }

    bool key_pressed(uint32_t key) {
        std::lock_guard<std::mutex> guard(_key_mutex);

        if (!_key_map[key])
            return false;

        if (_key_read[key])
            return false;

        _key_read[key] = true;

        return true;
    }
}
