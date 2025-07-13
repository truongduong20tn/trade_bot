#pragma once
#include <memory>
#include <thread>
#include <atomic>
#include "WebSocketClient.hpp"
#include "Strategy.hpp"
#include "TradeApiClient.hpp"
#include "ConfigLoader.hpp"

class Bot {
public:
    Bot(const std::string& configPath);
    void run();
    void stop();
private:
    std::unique_ptr<ConfigLoader> config_;
    std::unique_ptr<Strategy> strategy_;
    std::unique_ptr<TradeApiClient> apiClient_;
    std::unique_ptr<WebSocketClient> wsClient_;
    std::atomic<bool> running_;
    std::thread wsThread_;
};
