#pragma once
#include <memory>
#include <thread>
#include <atomic>
#include "WebSocketClient.hpp"
#include "Strategy.hpp"
#include "TradeApiClient.hpp"
#include "ConfigLoader.hpp"
#include "UserStreamClient.hpp"
#include "TradeLogger.hpp"

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
    std::unique_ptr<UserStreamClient> userStream_;
    std::atomic<bool> running_;
    std::thread wsThread_;
};
