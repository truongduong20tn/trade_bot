#include "WebSocketClient.hpp"
#include <nlohmann/json.hpp>
#include <iostream>
#include <thread>

WebSocketClient::WebSocketClient(const std::string& symbol, PriceCallback cb)
    : cb(cb) {
    std::string lowerSymbol = symbol;
    std::transform(lowerSymbol.begin(), lowerSymbol.end(), lowerSymbol.begin(), ::tolower);
    uri = "wss://stream.binance.com:9443/ws/" + lowerSymbol + "@trade";
    }

void WebSocketClient::run() {
    std::cout << "[INFO] Initializing WebSocket client\n";
    client.init_asio();
    client.set_access_channels(websocketpp::log::alevel::none);

    client.set_tls_init_handler([this](websocketpp::connection_hdl hdl)
                                { std::cout << "[INFO] TLS init handler triggered\n";
                                    return on_tls_init(hdl); });

    client.set_open_handler([](websocketpp::connection_hdl)
                            { std::cout << "[INFO] WebSocket connection opened\n";});

    client.set_fail_handler([](websocketpp::connection_hdl)
                            { std::cerr << "[ERROR] WebSocket connection failed\n"; });

    client.set_close_handler([](websocketpp::connection_hdl)
                             { std::cout << "[INFO] WebSocket connection closed\n"; });
    client.set_message_handler([this](websocketpp::connection_hdl, ws_client::message_ptr msg) {
        try {
            auto j = nlohmann::json::parse(msg->get_payload());
            if (j.contains("p")) {
                double price = std::stod(j["p"].get<std::string>());
                cb(price);
                auto now = std::chrono::steady_clock::now();
                if (now - last_print_time >= print_interval)
                {
                    std::cout << "[DATA] Price received: " << price << std::endl;
                    last_print_time = now;
                }
            }
            else
            {
                std::cout << "[DEBUG] Message received but no price: " << msg->get_payload() << "\n";
            }
        } catch (...) {
            std::cerr << "[ERROR] JSON parse failed: " << "\n";
        }
    });

    websocketpp::lib::error_code ec;
    auto con = client.get_connection(uri, ec);
    if (ec) {
        std::cerr << "WS error: " << ec.message() << "\n";
        return;
    }

    client.connect(con);

    std::thread([this]() {
        std::cout << "[INFO] Starting WebSocket event loop\n";
        client.run();
    }).detach();
}

WebSocketClient::context_ptr WebSocketClient::on_tls_init(websocketpp::connection_hdl) {
    context_ptr ctx = websocketpp::lib::make_shared<boost::asio::ssl::context>(
        boost::asio::ssl::context::tlsv12);

    try {
        ctx->set_options(boost::asio::ssl::context::default_workarounds |
                         boost::asio::ssl::context::no_sslv2 |
                         boost::asio::ssl::context::no_sslv3 |
                         boost::asio::ssl::context::single_dh_use);
    } catch (std::exception& e) {
        std::cerr << "TLS context error: " << e.what() << std::endl;
    }

    return ctx;
}


void WebSocketClient::stop() {
    client.stop();
}
