#pragma once
#include <iostream>
#include <nlohmann/json.hpp>
#include <string>

class ConfigLoader {
public:
    ConfigLoader(const std::string& path);
    std::string getApiKey() const;
    std::string getSecretKey() const;
    std::string getSymbol() const;
    std::string getQuantity() const;
    nlohmann::json getStrategyConfig() const;
    bool isTestMode() const;

private:
    nlohmann::json config;
};
