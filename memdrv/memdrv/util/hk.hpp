#ifndef _UTIL_HK_HPP_
#define _UTIL_HK_HPP_

#include "../defs.hpp"

extern "C" {
	NTSTATUS HkDetourFunction(void* target, void* detour, size_t size, void** original);
	NTSTATUS HkRestoreFunction(void* target, void* original);
}

#endif