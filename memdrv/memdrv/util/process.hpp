#ifndef _UTIL_PROCESS_HPP_
#define _UTIL_PROCESS_HPP_

#include "../defs.hpp"

namespace util {
    NTSTATUS GetProcessIdByName(PCWSTR processName, PHANDLE pid);
}

#endif