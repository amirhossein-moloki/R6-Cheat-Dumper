#ifndef _CORE_MEMORY_SERVICE_HPP_
#define _CORE_MEMORY_SERVICE_HPP_

#include "imemory_service.hpp"
#include <map>

namespace core {
    class MemoryService : public IMemoryService {
    public:
        MemoryService();
        ~MemoryService();

        bool initialize() override;
        void shutdown() override;
        std::string get_name() const override { return "MemoryService"; }

        bool attach(uint32_t pid) override;
        bool is_attached() const override;

        void clear_cache() override;
        void enable_caching(bool enable) override;

        bool read(uintptr_t address, void* buffer, size_t size) override;
        bool write(uintptr_t address, const void* buffer, size_t size) override;

        uintptr_t get_module_base(const wchar_t* module_name) override;
        bool has_write_access() const override;
        bool is_kernel_mode() const override;

    private:
        uint32_t m_pid;
        uint64_t m_handle;
        bool m_attached;
        bool m_caching_enabled;
        std::map<uintptr_t, uint64_t> m_cache;
    };
}

#endif