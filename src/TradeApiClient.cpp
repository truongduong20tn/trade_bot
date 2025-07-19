#include "TradeApiClient.hpp"
#include "Utils.hpp"
#include <openssl/hmac.h>
#include <curl/curl.h>
#include <iostream>
#include <sstream>
#include <iomanip>

TradeApiClient::TradeApiClient(std::string apiKey, std::string secretKey, std::string baseUrl)
 : apiKey_(std::move(apiKey)), secretKey_(std::move(secretKey)), baseUrl_(std::move(baseUrl)), allUSDT_(0.0), btcBalance_(0.0) {}

static std::string hmac_sha256(const std::string& key, const std::string& data) {
    unsigned char* digest = HMAC(EVP_sha256(),
        key.c_str(), key.size(),
        reinterpret_cast<const unsigned char*>(data.c_str()), data.size(),
        nullptr, nullptr);
    std::ostringstream oss;
    for (int i = 0; i < 32; ++i)
        oss << std::hex << std::setw(2) << std::setfill('0') << (int)digest[i];
    return oss.str();
}

void TradeApiClient::placeMarketOrder(const std::string& symbol,const std::string& side,const std::string& quantity) {
    if (allUSDT_ <= 10.0) {
        std::cout << "Khong du USDT de giao dich!!!";
        return;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    std::string url = baseUrl_ + "/v3/order";
    long long ts = current_timestamp_ms();
    std::ostringstream q;

    if (side == "BUY")
    {
        if (allUSDT_ <= 10.0)
        {
            std::cout << "[Trade] Không đủ USDT để mua BTC\n";
            return;
        }
        q << "symbol=" << symbol
          << "&side=BUY"
          << "&type=MARKET"
          << "&quoteOrderQty=" << std::fixed << std::setprecision(2) << allUSDT_
          << "&timestamp=" << ts;
    }
    else if (side == "SELL")
    {
        if (btcBalance_ <= 0.00001)
        {
            std::cout << "[Trade] Không đủ BTC để bán\n";
            return;
        }
        q << "symbol=" << symbol
          << "&side=SELL"
          << "&type=MARKET"
          << "&quantity=" << std::fixed << std::setprecision(6) << btcBalance_
          << "&timestamp=" << ts;
    }
    else
    {
        std::cerr << "[Trade] Invalid side: " << side << "\n";
        return;
    }

    std::string sig = hmac_sha256(secretKey_, q.str());
    std::string fullUrl = url + "?" + q.str() + "&signature=" + sig;

    CURL* curl = curl_easy_init();
    if (curl) {
        struct curl_slist* hdrs = nullptr;
        hdrs = curl_slist_append(hdrs, ("X-MBX-APIKEY: " + apiKey_).c_str());
        curl_easy_setopt(curl, CURLOPT_URL, fullUrl.c_str());
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, hdrs);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK)
            std::cerr << "CURL error: " << curl_easy_strerror(res) << "\n";
        curl_slist_free_all(hdrs);
        curl_easy_cleanup(curl);
    }
}

void TradeApiClient::printBalanceUSDT() {
    std::lock_guard<std::mutex> lock(mutex_);
    std::string url = baseUrl_ + "/v3/account";
    long long ts = current_timestamp_ms();
    std::ostringstream q;
    q << "timestamp=" << ts;

    std::string sig = hmac_sha256(secretKey_, q.str());
    std::string fullUrl = url + "?" + q.str() + "&signature=" + sig;
    CURL* curl = curl_easy_init();
    if (curl) {
        std::string response;
        struct curl_slist* hdrs = nullptr;
        std::string apiKeyHeader = "X-MBX-APIKEY: " + apiKey_;      
        hdrs = curl_slist_append(hdrs, apiKeyHeader.c_str());
        curl_easy_setopt(curl, CURLOPT_URL, fullUrl.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, hdrs);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            std::cerr << "[ERROR] CURL: " << curl_easy_strerror(res) << "\n";
        } else {
            try {
                auto j = nlohmann::json::parse(response);
                for (const auto& asset : j["balances"]) {
                    if (asset["asset"] == "USDT") {
                        std::cout << "[BALANCE] USDT Free: " << asset["free"]
                                  << ", Locked: " << asset["locked"] << "\n";
                        allUSDT_ = asset["free"].get<double>();
                        std::cout << "Set allUSDT_: " << allUSDT_;
                        break;
                    }
                }
            } catch (...) {
                std::cerr << "[ERROR] Failed to parse balance JSON\n";
            }
        }

        curl_slist_free_all(hdrs);
        curl_easy_cleanup(curl);
    }
}

void TradeApiClient::setBTCBalance(double btc) {
    std::lock_guard<std::mutex> lock(mutex_);
    std::cout << "btcBalance_ = " << btc << "\n";
    btcBalance_ = btc;
}

size_t TradeApiClient::write_callback(char* ptr, size_t size, size_t nmemb, void* userdata) {
    auto* response = static_cast<std::string*>(userdata);
    response->append(ptr, size * nmemb);
    return size * nmemb;
}

