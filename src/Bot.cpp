#include "Bot.hpp"
#include "WebSocketClient.hpp"
#include "UserStreamClient.hpp"
#include <iostream>
#define LOG_TRADE(action, symbol, quantity, price) \
    TradeLogger::instance().logTrade(action, symbol, quantity, price)

Bot::Bot(const std::string &configPath) : running_(true)
{
    config_ = std::make_unique<ConfigLoader>(configPath);
    strategy_.reset(Strategy::create(config_->getStrategyConfig()));
    apiClient_ = std::make_unique<TradeApiClient>(config_->getApiKey(), config_->getSecretKey(), config_->getBaseUrl());
    userStream_ = std::make_unique<UserStreamClient>(config_->getApiKey(), config_->getBaseUrl(), apiClient_.get());
    std::cout << "[CONFIG] Symbol: " << config_->getSymbol() << "\n";
    wsClient_ = std::make_unique<WebSocketClient>(
        config_->getSymbol(),
        [this](double price)
        {
            try
            {    
                if (strategy_ && apiClient_)
                {
                    if (strategy_->shouldBuy(price))
                    {
                        std::cout << "[Bot] Gửi lệnh BUY\n";
                        std::cout << "[WS] Price buy: " << price << "\n";
                        apiClient_->placeMarketOrder(config_->getSymbol(), "BUY", config_->getQuantity());
                        LOG_TRADE("BUY", config_->getSymbol(), config_->getQuantity(), price);
                    }
                    else if (strategy_->shouldSell(price))
                    {
                        std::cout << "[Bot] Gửi lệnh SELL\n";
                        std::cout << "[WS] Price sell: " << price << "\n";
                        apiClient_->placeMarketOrder(config_->getSymbol(), "SELL", config_->getQuantity());
                        LOG_TRADE("SELL", config_->getSymbol(), config_->getQuantity(), price);
                    }
                }
            }
            catch (const std::exception &e)
            {
                std::cerr << "[EXCEPTION] Trong price callback: " << e.what() << "\n";
            }
        });
    userStream_->start([this](const std::string &symbol, const std::string &side, double qty, double price)
                       {
    std::cout << "[UserStream] Đã " << (side == "BUY" ? "mua" : "bán") << " " << qty << " " << symbol << " tại giá " << price << "\n";
    if (side == "BUY" && strategy_) {
        strategy_->setLastBuyPrice(price); // Cập nhật giá mua thực tế
    }
    if (apiClient_) {
        apiClient_->printBalanceUSDT();  
    } });
}

void Bot::run() {  
    try
    {
        if (apiClient_)
        {
            apiClient_->printBalanceUSDT();
        }
        else
        {
            std::cerr << "apiClient_ is nullptr";
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "[EXCEPTION] printBalanceUSDT: " << e.what() << "\n";
    }
    wsThread_ = std::thread([&]()
                            { wsClient_->run(); });
    while (running_)
        std::this_thread::sleep_for(std::chrono::seconds(1));
    wsClient_->stop();
    wsThread_.join();
}

void Bot::stop() { 
    running_ = false;
    if (userStream_) {
        userStream_->stop();
    }
}
