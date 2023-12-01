#pragma once

#include <chrono>
namespace utils {
class Timer {
public:
    Timer() : m_startTimepoint(std::chrono::high_resolution_clock::now()) {}

    ~Timer() { Stop(); }

    uint32_t Stop() {
        auto endTimepoint = std::chrono::high_resolution_clock::now();
        auto start =
            std::chrono::time_point_cast<std::chrono::milliseconds>(m_startTimepoint).time_since_epoch().count();
        auto   end = std::chrono::time_point_cast<std::chrono::milliseconds>(endTimepoint).time_since_epoch().count();
        auto   duration = end - start;
        double ms       = duration * 0.001;
        return duration;
    }

private:
    std::chrono::time_point<std::chrono::high_resolution_clock> m_startTimepoint;
};
}  // namespace utils