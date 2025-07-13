#include "Strategy.hpp"
#include <iostream>

class PriceDropStrategy : public Strategy {
    double threshold_;
    double lastPrice_ = 0.0;

public:
    PriceDropStrategy(double t) : threshold_(t) {}
    bool shouldBuy(double price) override {
        if (lastPrice_ == 0.0) { lastPrice_ = price; return false; }
        if (price < lastPrice_ * (1.0 - threshold_ / 100.0)) {
            lastPrice_ = price;
            return true;
        }
        return false;
    }
};

Strategy* Strategy::create(const nlohmann::json& cfg) {
    std::string type = cfg["type"].get<std::string>();
    if (type == "price_drop") {
        double th = cfg["threshold_percent"].get<double>();
        return new PriceDropStrategy(th);
    }
    // TODO: Thêm các chiến lược khác tại đây
    std::cerr << "Strategy không hợp lệ: " << type << "\n";
    return nullptr;
}
