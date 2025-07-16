#include <iostream>
#include <vector>
#include <thread>
#include <memory>

#include "exchange/binance/binance.h"
#include "kitchen/strategy_manager.h"
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
    // load strategy manager
    LOG_INFO(g_logger, "Loading Strategy Manager...");
    degen_crypto::kitchen::StrategyManager strategy_manager;
    LOG_INFO(g_logger, "Done Loading Strategy Manager!");
    // load exchanges
    LOG_INFO(g_logger, "Loading Exchanges...");

    LOG_INFO(g_logger, "Loading Binance...");
    std::unique_ptr<binance::BinanceExchange> binance_engine = std::make_unique<binance::BinanceExchange>();
        
    std::jthread binance_worker([&binance_engine, &strategy_manager]() { 
        binance_engine->on_start(strategy_manager); 
        
        // Set up orderbook callbacks to notify strategies
        for (auto& [symbol, orderbook] : binance_engine->order_books_) {
            orderbook->set_orderbook_update_callback(
                [&strategy_manager](const std::string& exchange_name, const std::string& symbol, const order_book::OrderBook& orderbook) {
                    strategy_manager.notify_orderbook_update(exchange_name, symbol, orderbook);
                }
            );
        }

    });


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