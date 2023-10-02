#pragma once

#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <thread>
using namespace std::literals;

#define LOG(...) Logger::GetInstance().Log(__VA_ARGS__)
class Logger {
public:
    static Logger& GetInstance() {
        static Logger instance;
        return instance;
    }

    template <typename... Args>
    void Log(Args&&... args) {
        std::lock_guard<std::mutex> lock(mutex_);
        CheckForDateChange();

        std::ofstream logfile(logFilePath_, std::ios_base::app);
        if (!logfile.is_open()) {
            std::cout << "Failed to open log file: " << logFilePath_ << std::endl;
            return;
        }

        logfile << GetTimeStamp() << ": ";
        LogHelper(logfile, std::forward<Args>(args)...);
        logfile << std::endl;

        logfile.close();
    }

    void SetTimestamp(const std::chrono::system_clock::time_point& timestamp) {
        std::lock_guard<std::mutex> lock(mutex_);
        manual_ts_ = timestamp;
        UpdateLogFilePath();
    }

private:
    std::string logFilePath_;
    std::optional<std::chrono::system_clock::time_point> manual_ts_;
    std::mutex mutex_;

    Logger() : manual_ts_(std::chrono::system_clock::now()) { UpdateLogFilePath(); }

    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    template <typename T, typename... Args>
    void LogHelper(std::ostream& os, T&& arg, Args&&... args) {
        os << std::boolalpha << std::forward<T>(arg);
        LogHelper(os, std::forward<Args>(args)...);
    }

    void LogHelper(std::ostream& os) {
        // Base case
    }

    std::string GetTimeStamp() const {
        auto time = std::chrono::system_clock::to_time_t(*manual_ts_);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
        return ss.str();
    }

    void UpdateLogFilePath() {
        auto time = std::chrono::system_clock::to_time_t(*manual_ts_);
        std::stringstream ss;
        ss << "/var/log/sample_log_" << std::put_time(std::localtime(&time), "%Y_%m_%d") << ".log";
        logFilePath_ = ss.str();
    }

    void CheckForDateChange() {
        auto currentTime = std::chrono::system_clock::now();
        if (currentTime - *manual_ts_ >= std::chrono::hours(24)) {
            manual_ts_ = currentTime;
            UpdateLogFilePath();
        }
    }
};
// class Logger {
//     auto GetTime() const {
//         if (manual_ts_) {
//             return *manual_ts_;
//         }

//         return std::chrono::system_clock::now();
//     }

//     auto GetTimeStamp() const {
//         const auto now = GetTime();
//         const auto t_c = std::chrono::system_clock::to_time_t(now);
//         return std::put_time(std::localtime(&t_c), "%F %T");
//     }

//     // Для имени файла возьмите дату с форматом "%Y_%m_%d"
//     std::string GetFileTimeStamp() const;

//     Logger() = default;
//     Logger(const Logger&) = delete;

// public:
//     static Logger& GetInstance() {
//         static Logger obj;
//         return obj;
//     }

//     // Выведите в поток все аргументы.
//     template<class... Ts>
//     void Log(const Ts&... args);

//     // Установите manual_ts_. Учтите, что эта операция может выполняться
//     // параллельно с выводом в поток, вам нужно предусмотреть
//     // синхронизацию.
//     void SetTimestamp(std::chrono::system_clock::time_point ts);

// private:
//     std::optional<std::chrono::system_clock::time_point> manual_ts_;
// };
