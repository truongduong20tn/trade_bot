#include "Bot.hpp"
#include <csignal>
#include <memory>

std::unique_ptr<Bot> bot;

void on_sigint(int) {
    if (bot) bot->stop();
}

int main() {
    std::signal(SIGINT, on_sigint);
    bot = std::make_unique<Bot>("config.json");
    bot->run();
    return 0;
}
