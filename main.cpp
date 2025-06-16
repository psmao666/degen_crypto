#include <iostream>
#include <fmt/core.h>
#include <vector>
#include <thread>
#include <memory>

#include "exchange/binance/binance.h"

using namespace degen_crypto::exchange;

int main() { // NOLINT
    fmt::print("Degen Crypto Bootstrapping...\n");
    // load glassnode
    fmt::print("Loading Glassnode APIs...\n");
    fmt::print("Done Loading Glassnode APIs!\n");
    // load exchanges
    fmt::print("Loading Exchanges...\n");
    
    fmt::print("Loading Binance...\n");
    std::unique_ptr<ExchangeEngine<binance::BinanceExchange>> binance_engine = std::make_unique<binance::BinanceExchange>();
    std::jthread binance_worker([&binance_engine]() { binance_engine->on_start(); });

    fmt::print("Done Loading Exchanges!\n");
    // load indicators
    fmt::print("Loading Indicators...\n");
    fmt::print("Done Loading Indicators!\n");
    // load strategies
    fmt::print("Loading Strategies...\n");
    fmt::print("Done Loading Strategies!\n");
    fmt::print("--------------------------------\n");
    fmt::print("Degen Crypto Running...\n");
    fmt::print("--------------------------------\n");

    return 0;
}