#include <iostream>
#include <vector>
#include <thread>
#include <memory>

#include "exchange/binance/binance.h"
#include "exchange/bybit/bybit.h"
#include "exchange/constants.h"
#include "exchange/position_manager.h"
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

    degen_crypto::exchange::PositionManager position_manager;
    // load exchanges
    LOG_INFO(g_logger, "Loading Exchanges...");

#if defined(BINANCE)
    LOG_INFO(g_logger, "Loading Binance...");
    degen_crypto::exchange::binance::BinanceExchange exchange_engine;
    std::jthread exchange_worker([&exchange_engine, &strategy_manager, &position_manager]() {
        exchange_engine.on_start(strategy_manager,
            [&position_manager, &strategy_manager, &exchange_engine](const std::unordered_map<std::string, double>& balances) {
                position_manager.add_exchange(degen_crypto::exchange::binance::constants::EXCHANGE_NAME, balances);
                strategy_manager.set_position_manager(&position_manager);
                strategy_manager.set_exchange_engine(&exchange_engine);
            },
            [&position_manager](const std::unordered_map<std::string, double>& balances) {
                position_manager.refresh(degen_crypto::exchange::binance::constants::EXCHANGE_NAME, balances);
            }
        );
    });
#elif defined(BYBIT)
    LOG_INFO(g_logger, "Loading Bybit...");
    degen_crypto::exchange::bybit::BybitExchange exchange_engine;
    std::jthread exchange_worker([&exchange_engine, &strategy_manager, &position_manager]() {
        exchange_engine.on_start(strategy_manager,
            [&position_manager, &strategy_manager, &exchange_engine](const std::unordered_map<std::string, double>& balances) {
                position_manager.add_exchange(degen_crypto::exchange::bybit::constants::EXCHANGE_NAME, balances);
                strategy_manager.set_position_manager(&position_manager);
                strategy_manager.set_exchange_engine(&exchange_engine);
            },
            [&position_manager](const std::unordered_map<std::string, double>& balances) {
                position_manager.refresh(degen_crypto::exchange::bybit::constants::EXCHANGE_NAME, balances);
            }
        );
    });
#else
    #error "No exchange defined. Please compile with -DBINANCE=ON or -DBYBIT=ON."
#endif

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