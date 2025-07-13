#pragma once
#include <nlohmann/json.hpp>

class Strategy {
public:
    virtual ~Strategy() = default;
    virtual bool shouldBuy(double price) = 0;
    static Strategy* create(const nlohmann::json& config);
};
