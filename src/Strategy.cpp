#include "Strategy.hpp"
#include <iostream>

class PriceDropStrategy : public Strategy {
    double threshold_;           // % drop để mua
    double takeProfitRatio_;     // % lời để bán
    double stopLossRatio_;       // % lỗ để bán
    double lastPrice_ = 0.0;

public:
    PriceDropStrategy(double threshold, double tp, double sl)
        : threshold_(threshold), takeProfitRatio_(tp / 100.0), stopLossRatio_(sl / 100.0) {}
    bool shouldBuy(double price) override {
        if (lastPrice_ == 0.0) { lastPrice_ = price; return false; }
        if (price < lastPrice_ * (1.0 - threshold_ / 100.0)) {
            lastPrice_ = price;
            lastBuyPrice_ = price;
            hasBought_ = true;
            return true;
        }
        return false;
    }
    bool shouldSell(double price) override
    {
        if (!hasBought_)
            return false;
        if (price >= lastBuyPrice_ * (1.0 + takeProfitRatio_))
        {
            std::cout << "[Strategy] Đạt chốt lời tại giá " << price << "\n";
            hasBought_ = false;
            return true;
        }

        if (price <= lastBuyPrice_ * (1.0 - stopLossRatio_))
        {
            std::cout << "[Strategy] Đạt cắt lỗ tại giá " << price << "\n";
            hasBought_ = false;
            return true;
        }
        return false;
    }
};

Strategy* Strategy::create(const nlohmann::json& cfg) {
    std::string type = cfg["type"].get<std::string>();
    if (type == "price_drop") {
        double th = cfg["threshold_percent"].get<double>();
        double tp = cfg["take_profit_percent"].get<double>();
        double tl = cfg["stop_loss_percent"].get<double>();
        return new PriceDropStrategy(th, tp, tl);
    }
    // TODO: Thêm các chiến lược khác tại đây
    std::cerr << "Strategy không hợp lệ: " << type << "\n";
    return nullptr;
}
