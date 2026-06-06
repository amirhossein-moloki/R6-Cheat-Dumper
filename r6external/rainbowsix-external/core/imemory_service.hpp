#ifndef _CORE_IMEMORY_SERVICE_HPP_
#define _CORE_IMEMORY_SERVICE_HPP_

#include <cstdint>
#include <vector>
#include <map>
#include "iservice.hpp"

namespace core {
class IMemoryService : public IService {
public:
    virtual bool attach(uint32_t pid) = 0;
    virtual bool is_attached() const = 0;

    virtual bool read(uintptr_t address, void* buffer, size_t size) = 0;
    virtual bool write(uintptr_t address, const void* buffer, size_t size) = 0;

    // Caching
    virtual void clear_cache() = 0;
    virtual void enable_caching(bool enable) = 0;

    template <typename T>
    T read(uintptr_t address) {
        T result = {};
        read(address, &result, sizeof(T));
        return result;
    }

    template <typename T>
    bool write(uintptr_t address, const T& value) {
        return write(address, &value, sizeof(T));
    }

    virtual uintptr_t get_module_base(const wchar_t* module_name) = 0;
    virtual bool has_write_access() const = 0;
    virtual bool is_kernel_mode() const = 0;
};
} // namespace core

#endif