#include "Bot.hpp"
#include "WebSocketClient.hpp"
#include <iostream>

Bot::Bot(const std::string& configPath) : running_(true) {
    config_ = std::make_unique<ConfigLoader>(configPath);
    strategy_.reset(Strategy::create(config_->getStrategyConfig()));
    apiClient_ = std::make_unique<TradeApiClient>(config_->getApiKey(),config_->getSecretKey(), config_->getBaseUrl());
    std::cout << "[CONFIG] Symbol: " << config_->getSymbol() << "\n";
    wsClient_ = std::make_unique<WebSocketClient>(
        config_->getSymbol(),
        [this](double price){
            std::cout << "[WS] Giá: " << price << "\n";
            if (strategy_ && strategy_->shouldBuy(price)) {
                std::cout << "[Bot] Gửi lệnh BUY\n";
                apiClient_->placeMarketOrder(config_->getSymbol(), "BUY", config_->getQuantity());
            }
        });
}

void Bot::run() {
    wsThread_ = std::thread([&](){ wsClient_->run(); });
    while (running_) std::this_thread::sleep_for(std::chrono::seconds(1));
    wsClient_->stop();
    wsThread_.join();
}

void Bot::stop() { running_ = false; }
