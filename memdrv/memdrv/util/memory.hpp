#ifndef _UTIL_MEMORY_HPP_
#define _UTIL_MEMORY_HPP_

#include "../defs.hpp"

namespace util {
    NTSTATUS GetProcessModuleBase(PEPROCESS process, PCWSTR moduleName, PUINT64 baseAddress);
}

#endif