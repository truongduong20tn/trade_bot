#pragma once
#include <nlohmann/json.hpp>

class Strategy {
public:
    virtual ~Strategy() = default;
    virtual bool shouldBuy(double price) = 0;
    virtual bool shouldSell(double price) = 0;
    static Strategy* create(const nlohmann::json& config);
    void setLastBuyPrice(double price) { lastBuyPrice_ = price; hasBought_ = true; }
protected:
    double lastBuyPrice_ = 0.0;
    bool hasBought_ = false;
};
