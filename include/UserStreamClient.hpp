#pragma once
#include <string>
#include <thread>
#include <atomic>
#include <functional>
#include <websocketpp/client.hpp>
#include <websocketpp/config/asio_client.hpp>
#include "TradeApiClient.hpp"

class UserStreamClient {
public:
    using OrderFilledCallback = std::function<void(const std::string& symbol, const std::string& side, double quantity, double price)>;

    UserStreamClient(const std::string& apiKey, const std::string& baseUrl, TradeApiClient* apiClient);
    ~UserStreamClient();

    void start(OrderFilledCallback onOrderFilled);
    void stop();
    static size_t write_callback(char* ptr, size_t size, size_t nmemb, void* userdata);

private:
    using WebSocketClient = websocketpp::client<websocketpp::config::asio_tls_client>;
    WebSocketClient client;
    std::string apiKey_;
    std::string baseUrl_;
    std::string listenKey_;
    std::atomic<bool> running_;
    std::thread wsThread_;
    OrderFilledCallback onOrderFilled_;
    TradeApiClient* apiClient_;

    void fetchListenKey();
    void runWebSocket();
    void handleMessage(const std::string& message);
};
