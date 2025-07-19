#pragma once

#include <string>
#include <fstream>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <mutex>

class TradeLogger {
public:
    static TradeLogger& instance() {
        static TradeLogger logger("trade_log.txt");
        return logger;
    }

    void logTrade(const std::string& action,
                  const std::string& symbol,
                  const std::string& quantity,
                  double price)
    {
        std::lock_guard<std::mutex> lock(mutex_);

        auto now = std::chrono::system_clock::now();
        std::time_t t_c = std::chrono::system_clock::to_time_t(now);

        logFile << "[" << std::put_time(std::localtime(&t_c), "%F %T") << "] "
                << action << " " << quantity << " " << symbol
                << " at price " << price << "\n";

        logFile.flush();
    }

private:
    explicit TradeLogger(const std::string& filename)
        : logFile(filename, std::ios::app)
    {
        if (!logFile.is_open()) {
            throw std::runtime_error("Cannot open log file: " + filename);
        }
    }

    std::ofstream logFile;
    std::mutex mutex_;
};
