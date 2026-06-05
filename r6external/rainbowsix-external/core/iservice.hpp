#ifndef _CORE_ISERVICE_HPP_
#define _CORE_ISERVICE_HPP_

#include <string>

namespace core {
    class IService {
    public:
        virtual ~IService() = default;
        virtual bool initialize() = 0;
        virtual void shutdown() = 0;
        virtual std::string get_name() const = 0;
    };
}

#endif