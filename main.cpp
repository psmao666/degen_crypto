#include <iostream>
#include <vector>
#include <thread>
#include <memory>

#include "exchange/binance/binance.h"
#include "common/utils/logger.h"

using namespace degen_crypto::exchange;
using namespace degen_crypto::logger;

int main() { // NOLINT
    // Initialize the global logger
    init_logger();
    
    LOG_INFO(g_logger, "Degen Crypto Bootstrapping...");
    // load glassnode
    LOG_INFO(g_logger, "Loading Glassnode APIs...");
    LOG_INFO(g_logger, "Done Loading Glassnode APIs!");
    // load exchanges
    LOG_INFO(g_logger, "Loading Exchanges...");
    
    LOG_INFO(g_logger, "Loading Binance...");
    std::unique_ptr<ExchangeEngine<binance::BinanceExchange>> binance_engine = std::make_unique<binance::BinanceExchange>();
    std::jthread binance_worker([&binance_engine]() { binance_engine->on_start(); });

    LOG_INFO(g_logger, "Done Loading Exchanges!");
    // load indicators
    LOG_INFO(g_logger, "Loading Indicators...");
    LOG_INFO(g_logger, "Done Loading Indicators!");
    // load strategies
    LOG_INFO(g_logger, "Loading Strategies...");
    LOG_INFO(g_logger, "Done Loading Strategies!");
    LOG_INFO(g_logger, "--------------------------------");
    LOG_INFO(g_logger, "Degen Crypto Running...");
    LOG_INFO(g_logger, "--------------------------------");

    return 0;
}