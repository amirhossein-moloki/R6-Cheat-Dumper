#ifndef _OVERLAY_RENDERER_RENDERER_HPP_
#define _OVERLAY_RENDERER_RENDERER_HPP_

#include <d3d11.h>
#include <dxgi.h>

namespace overlay::renderer {
    extern ID3D11Device* device;
    extern ID3D11DeviceContext* context;
    extern ID3D11RenderTargetView* target_view;
    extern IDXGISwapChain* swap_chain;

    bool initialized();
    bool create(HWND hwnd);
    void destroy();
    void begin_frame();
    void end_frame();
}

#endif