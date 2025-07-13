#pragma once
#include <string>

class TradeApiClient {
public:
    TradeApiClient(std::string apiKey, std::string secretKey, std::string baseUrl);
    void placeMarketOrder(const std::string& symbol,const std::string& side,const std::string& quantity);
    private:
    std::string apiKey_;     
    std::string secretKey_; 
    std::string baeUrl_; 
};
