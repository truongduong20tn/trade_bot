#include "Bot.hpp"
#include <csignal>
#include <memory>
#include <curl/curl.h>

std::unique_ptr<Bot> bot;

void on_sigint(int) {
    if (bot) bot->stop();
}

int main() {
    curl_global_init(CURL_GLOBAL_ALL);
    std::signal(SIGINT, on_sigint);
    bot = std::make_unique<Bot>("config.json");
    bot->run();
    return 0;
}
