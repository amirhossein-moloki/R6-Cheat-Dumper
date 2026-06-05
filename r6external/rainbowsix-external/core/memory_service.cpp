#include "memory_service.hpp"
#include "../driver/driver.hpp"
#include "logger.hpp"
#include <cstring>

namespace core {
    MemoryService::MemoryService() : m_pid(0), m_attached(false), m_caching_enabled(false) {}

    MemoryService::~MemoryService() {
        shutdown();
    }

    bool MemoryService::initialize() {
        if (!driver::initialize()) {
            LOG_ERROR("Failed to initialize driver interface.");
            return false;
        }
        return true;
    }

    void MemoryService::shutdown() {
        if (driver::g_interface) {
            driver::g_interface->Shutdown();
        }
    }

    bool MemoryService::attach(uint32_t pid) {
        m_pid = pid;
        m_handle = driver::open_process(pid);
        m_attached = (m_handle != 0);
        if (m_attached) {
            clear_cache();
        }
        return m_attached;
    }

    bool MemoryService::is_attached() const { return m_attached; }

    void MemoryService::clear_cache() {
        m_cache.clear();
    }

    void MemoryService::enable_caching(bool enable) {
        m_caching_enabled = enable;
        if (!enable) clear_cache();
    }

    bool MemoryService::read(uintptr_t address, void* buffer, size_t size) {
        if (!m_attached) return false;

        if (m_caching_enabled && size <= 8) {
            auto it = m_cache.find(address);
            if (it != m_cache.end()) {
                memcpy(buffer, &it->second, size);
                return true;
            }
        }

        bool success = driver::read_memory(m_handle, address, static_cast<uint8_t*>(buffer), static_cast<uint32_t>(size));

        if (success && m_caching_enabled && size <= 8) {
            uint64_t val = 0;
            memcpy(&val, buffer, size);
            m_cache[address] = val;
        }

        return success;
    }

    bool MemoryService::write(uintptr_t address, const void* buffer, size_t size) {
        if (!m_attached || !has_write_access()) return false;

        bool success = driver::write_memory(m_handle, address, static_cast<const uint8_t*>(buffer), static_cast<uint32_t>(size));

        if (success && m_caching_enabled) {
            m_cache.erase(address);
        }

        return success;
    }

    uintptr_t MemoryService::get_module_base(const wchar_t* module_name) {
        if (!m_attached) return 0;
        return driver::get_module_base(m_handle, module_name);
    }

    bool MemoryService::has_write_access() const {
        return driver::g_has_write_access;
    }

    bool MemoryService::is_kernel_mode() const {
        return !driver::g_user_mode;
    }
}
