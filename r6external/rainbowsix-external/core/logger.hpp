#ifndef _CORE_LOGGER_HPP_
#define _CORE_LOGGER_HPP_

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <memory>
#include <string>

namespace core {
    class Logger {
    public:
        static void initialize(const std::string& log_file = "technical_suite.log");
        static void shutdown();

        static std::shared_ptr<spdlog::logger> get_logger() { return s_logger; }

    private:
        static std::shared_ptr<spdlog::logger> s_logger;
    };
}

#define LOG_TRACE(...) core::Logger::get_logger()->trace(__VA_ARGS__)
#define LOG_DEBUG(...) core::Logger::get_logger()->debug(__VA_ARGS__)
#define LOG_INFO(...)  core::Logger::get_logger()->info(__VA_ARGS__)
#define LOG_WARN(...)  core::Logger::get_logger()->warn(__VA_ARGS__)
#define LOG_ERROR(...) core::Logger::get_logger()->error(__VA_ARGS__)
#define LOG_CRITICAL(...) core::Logger::get_logger()->critical(__VA_ARGS__)

// Enterprise Severity Tagging
#define LOG_P0(...) core::Logger::get_logger()->critical("[P0-CRITICAL] " __VA_ARGS__)
#define LOG_P1(...) core::Logger::get_logger()->error("[P1-HIGH] " __VA_ARGS__)
#define LOG_P2(...) core::Logger::get_logger()->warn("[P2-MEDIUM] " __VA_ARGS__)
#define LOG_P3(...) core::Logger::get_logger()->info("[P3-LOW] " __VA_ARGS__)

#endif
