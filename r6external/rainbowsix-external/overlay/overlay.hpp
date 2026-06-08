#ifndef _OVERLAY_OVERLAY_HPP_
#define _OVERLAY_OVERLAY_HPP_

#include <Windows.h>

namespace overlay {
    void enable();
    void disable();
    bool should_close();
    HWND get_window_handle();
}

#endif
