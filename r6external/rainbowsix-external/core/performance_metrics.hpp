#ifndef _CORE_PERFORMANCE_METRICS_HPP_
#define _CORE_PERFORMANCE_METRICS_HPP_

#include <chrono>
#include <string>
#include <deque>
#include <numeric>
#include <map>

namespace core {
    class PerformanceMetrics {
    public:
        static PerformanceMetrics& instance() {
            static PerformanceMetrics inst;
            return inst;
        }

        void start_frame() {
            m_frame_start = std::chrono::high_resolution_clock::now();
        }

        void end_frame() {
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - m_frame_start).count();

            m_latencies.push_back(duration);
            if (m_latencies.size() > 1000) {
                m_latencies.pop_front();
            }
        }

        double get_average_latency() const {
            if (m_latencies.empty()) return 0.0;
            return std::accumulate(m_latencies.begin(), m_latencies.end(), 0.0) / m_latencies.size();
        }

        void track_event(const std::string& name, long long duration_us) {
            auto& latencies = m_event_latencies[name];
            latencies.push_back(duration_us);
            if (latencies.size() > 100) {
                latencies.pop_front();
            }
        }

        std::map<std::string, double> get_event_averages() const {
            std::map<std::string, double> averages;
            for (const auto& [name, latencies] : m_event_latencies) {
                if (latencies.empty()) continue;
                averages[name] = std::accumulate(latencies.begin(), latencies.end(), 0.0) / latencies.size();
            }
            return averages;
        }

    private:
        PerformanceMetrics() = default;
        std::chrono::time_point<std::chrono::high_resolution_clock> m_frame_start;
        std::deque<long long> m_latencies;
        std::map<std::string, std::deque<long long>> m_event_latencies;
    };

    class ScopedTimer {
    public:
        ScopedTimer(const std::string& name) : m_name(name), m_start(std::chrono::high_resolution_clock::now()) {}
        ~ScopedTimer() {
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - m_start).count();
            PerformanceMetrics::instance().track_event(m_name, duration);
        }
    private:
        std::string m_name;
        std::chrono::time_point<std::chrono::high_resolution_clock> m_start;
    };
}

#endif
