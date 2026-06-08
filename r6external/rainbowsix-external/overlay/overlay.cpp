#include "overlay.hpp"
#include "../core/logger.hpp"
#include "../core/cheat_context.hpp"
#include "renderer/renderer.hpp"
#include "input/input.hpp"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_dx11.h"
#include "imgui/imgui_impl_win32.h"
#include "../game/game_util.h"
#include "../features/visuals/visuals.hpp"
#include "imgui/font.h"
#include "ck.hpp"
#include <dwmapi.h>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace overlay {
    HWND h_window = nullptr;
    bool b_should_close = false;

    LRESULT CALLBACK window_procedure(HWND window, UINT message, WPARAM w_param, LPARAM l_param) {
        if (ImGui_ImplWin32_WndProcHandler(window, message, w_param, l_param))
            return true;

        switch (message) {
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        case WM_KEYDOWN:
            if (w_param == VK_DELETE) {
                b_should_close = true;
            }
            break;
        }

        return DefWindowProc(window, message, w_param, l_param);
    }

    bool create_window() {
        WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, window_procedure, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, "TechnicalSuiteOverlay", NULL };
        RegisterClassEx(&wc);

        int screen_width = GetSystemMetrics(SM_CXSCREEN);
        int screen_height = GetSystemMetrics(SM_CYSCREEN);

        h_window = CreateWindowEx(
            WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED | WS_EX_TOOLWINDOW,
            wc.lpszClassName, "Technical Suite Overlay",
            WS_POPUP,
            0, 0, screen_width, screen_height,
            NULL, NULL, wc.hInstance, NULL
        );

        if (!h_window) {
            LOG_ERROR("Failed to create overlay window.");
            return false;
        }

        SetLayeredWindowAttributes(h_window, RGB(0, 0, 0), 0, LWA_COLORKEY);

        MARGINS margins = { -1 };
        DwmExtendFrameIntoClientArea(h_window, &margins);

        ShowWindow(h_window, SW_SHOWDEFAULT);
        UpdateWindow(h_window);

        return true;
    }

    void enable() {
        LOG_INFO("Initializing Standalone Overlay...");

        if (!create_window()) {
            exit(1);
        }

        if (!renderer::create(h_window)) {
            LOG_ERROR("Failed to initialize renderer for overlay.");
            exit(1);
        }

        ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(hck_compressed_data, hck_compressed_size, 18);

        input::enable();
        LOG_INFO("Overlay enabled successfully.");
    }

    void disable() {
        input::disable();
        renderer::destroy();
        if (h_window) {
            DestroyWindow(h_window);
            UnregisterClass("TechnicalSuiteOverlay", GetModuleHandle(NULL));
        }
    }

    bool should_close() {
        return b_should_close;
    }

    HWND get_window_handle() {
        return h_window;
    }
}
