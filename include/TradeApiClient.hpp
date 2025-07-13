#pragma once
#include <string>
#include <nlohmann/json.hpp>
#include <mutex>

class TradeApiClient {
public:
    TradeApiClient(std::string apiKey, std::string secretKey, std::string baseUrl);
    void placeMarketOrder(const std::string& symbol,const std::string& side,const std::string& quantity);
    void printBalanceUSDT();
    static size_t write_callback(char* ptr, size_t size, size_t nmemb, void* userdata);
    private:
    std::string apiKey_;     
    std::string secretKey_; 
    std::string baseUrl_; 
    std::mutex mutex_;
};
