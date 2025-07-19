#include "UserStreamClient.hpp"
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <iostream>

using json = nlohmann::json;


size_t UserStreamClient::write_callback(char* ptr, size_t size, size_t nmemb, void* userdata) {
    auto* response = static_cast<std::string*>(userdata);
    response->append(ptr, size * nmemb);
    return size * nmemb;
}

UserStreamClient::UserStreamClient(const std::string& apiKey, const std::string& baseUrl, TradeApiClient* apiClient)
    : apiKey_(apiKey), baseUrl_(baseUrl), running_(false), apiClient_(apiClient) {}

UserStreamClient::~UserStreamClient() {
    stop();
}

void UserStreamClient::start(OrderFilledCallback cb) {
    std::cout << "[UserStream] start \n";
    onOrderFilled_ = cb;
    fetchListenKey();
    running_ = true;
    wsThread_ = std::thread(&UserStreamClient::runWebSocket, this);
}

void UserStreamClient::stop() {
    running_ = false;
    client.stop();               // <-- Dừng kết nối WebSocket
    client.stop_perpetual();     // <-- Dừng vòng lặp run()
    if (wsThread_.joinable()) wsThread_.join();
}

void UserStreamClient::fetchListenKey() {
    std::cout << "[UserStream] fetchListenKey start \n";
    CURL* curl = curl_easy_init();
    if (!curl) {
        std::cout << "[UserStream] fetchListenKey null curl \n";
        return;
    }
    std::string url = baseUrl_ + "/v3/userDataStream";
    std::string response;
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, ("X-MBX-APIKEY: " + apiKey_).c_str());
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
    curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "");
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, 0L);
    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        std::cerr << "[CURL ERROR] " << curl_easy_strerror(res) << "\n";
    } else {
        auto j = json::parse(response);
        listenKey_ = j["listenKey"];
    }

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
}

void UserStreamClient::runWebSocket() {
    client.init_asio();
    client.set_access_channels(websocketpp::log::alevel::none);
    client.set_tls_init_handler([](websocketpp::connection_hdl) {
        return websocketpp::lib::make_shared<websocketpp::lib::asio::ssl::context>(websocketpp::lib::asio::ssl::context::sslv23);
    });

    client.set_message_handler([this](websocketpp::connection_hdl, WebSocketClient::message_ptr msg) {
        handleMessage(msg->get_payload());
    });

    std::string uri = "wss://stream.binance.com:9443/ws/" + listenKey_;
    websocketpp::lib::error_code ec;
    auto conn = client.get_connection(uri, ec);
    if (ec) {
        std::cerr << "WebSocket error: " << ec.message() << "\n";
        return;
    }

    client.connect(conn);
    std::cout << "[UserStream] WebSocket loop started\n";
    client.run();
}

void UserStreamClient::handleMessage(const std::string& message) {
    auto j = json::parse(message);
    std::cout << "[UserStream] Received message: " << message << "\n";
    if (j.contains("e") && j["e"] == "executionReport") {
        std::string status = j["X"];
        if (status == "FILLED") {
            std::string symbol = j["s"];
            std::string side = j["S"];
            double quantity = std::stod(j["z"].get<std::string>());
            double price = std::stod(j["L"].get<std::string>());
            if (onOrderFilled_) {
                onOrderFilled_(symbol, side, quantity, price);
            }
        }
    } else if (j["e"] == "outboundAccountPosition") {
        for (const auto& asset : j["B"]) {
            if (asset["a"] == "BTC") {
                double btc = std::stod(asset["f"].get<std::string>());
                apiClient_->setBTCBalance(btc);  // cập nhật vào client
                break;
            }
        }
    }
}
