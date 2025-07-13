#pragma once
#include <functional>
#include <string>
#include <boost/asio.hpp>
#include <websocketpp/config/asio_client.hpp>
#include <websocketpp/client.hpp>
#include <boost/asio/ssl/context.hpp>
#include <boost/asio/ssl.hpp>

class WebSocketClient {
public:
    using PriceCallback = std::function<void(double)>;
    WebSocketClient(const std::string& symbol, PriceCallback cb);
    void run();
    void stop();

private:
    using ws_client = websocketpp::client<websocketpp::config::asio_tls_client>;
    using context_ptr = websocketpp::lib::shared_ptr<boost::asio::ssl::context>;
    context_ptr on_tls_init(websocketpp::connection_hdl hdl);
    ws_client client;
    websocketpp::connection_hdl hdl;
    std::string uri;
    PriceCallback cb;
    bool running = false;
    std::chrono::steady_clock::time_point last_print_time = std::chrono::steady_clock::now();
    const std::chrono::seconds print_interval = std::chrono::seconds(10);
};
