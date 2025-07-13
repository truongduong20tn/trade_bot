#include "ConfigLoader.hpp"
#include <fstream>

ConfigLoader::ConfigLoader(const std::string &path)
{
    std::ifstream f("config.json");
    if (!f)
    {
        std::cerr << "Không mở được file config.json\n";
        return;
    }
    if (f.peek() == std::ifstream::traits_type::eof())
    {
        std::cerr << "File config.json rỗng\n";
        return;
    }
    config = nlohmann::json::parse(f);
}

std::string ConfigLoader::getApiKey() const { return config["api_key"].get<std::string>(); }
std::string ConfigLoader::getSecretKey() const { return config["secret_key"].get<std::string>(); }
std::string ConfigLoader::getSymbol() const { return config["symbol"].get<std::string>(); }
bool ConfigLoader::isTestMode() const { return config["isTestMode"].get<bool>(); }
std::string ConfigLoader::getQuantity() const { return config["quantity"].get<std::string>(); }
std::string ConfigLoader::getBaseUrl() const { return isTestMode() ? "https://testnet.binance.vision/api" : "https://api.binance.com/api"; }
nlohmann::json ConfigLoader::getStrategyConfig() const { return config["strategy"]; }
