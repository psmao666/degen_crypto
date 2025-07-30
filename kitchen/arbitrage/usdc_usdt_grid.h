#pragma once

#include "../../kitchen/strategy_manager.h"
#include "../../kitchen/strategy_config.h"
#include "nlohmann/json.hpp"
#include "../../exchange/constants.h"
#include "quill/LogMacros.h"

#include <common/utils/logger.h>
#include <common/utils/compare_case_insensitive.h>


namespace degen_crypto { namespace kitchen { namespace arbitrage {

class StrategyUSDCUSDTGrid : public Strategy<StrategyUSDCUSDTGrid> {
public:
    StrategyUSDCUSDTGrid() = default;
    ~StrategyUSDCUSDTGrid() = default;

    bool on_start() { run(); return true; }
    bool on_shutdown() { return true; }
    bool load_params(const nlohmann::json& params) { 
        return config_.load_params(params);
    }

    const char* strategy_name() const { return "USDC-USDT_Grid"; }
    void run() { }

    void on_orderbook_update_callback(const std::string& exchange_name, const std::string& symbol, const order_book_t& orderbook) {
        if (!common::utils::compare_str_insensitve(symbol, exchange::binance::constants::USDC_USDT)) return;

        auto& portfolio = position_manager().portfolio()[exchange_name];
        constexpr auto& USDC = exchange::binance::constants::USDC;
        constexpr auto& USDT = exchange::binance::constants::USDT;
        auto usdc = portfolio.get_position(USDC);
        auto usdt = portfolio.get_position(USDT);
        auto& engine = exchange_engine();
        const auto best_bid = orderbook.best_bid().price();
        const auto best_ask = orderbook.best_ask().price();
        // LOG_INFO(logger::g_logger, "{}::current best bid at {}, current best ask at {}, usdt: {}, usdc: {}", strategy_name(), best_bid, best_ask, usdt, usdc); 

        if (best_ask <= config_.buy_levels[0].price && usdt > 0) {
            const auto quantity = static_cast<int>(usdt * config_.buy_levels[0].buy_capital_ratio / best_ask);
            if (quantity <= 6) {
                LOG_INFO(logger::g_logger, "StrategyUSDCUSDTGrid::Not enough notional, not buying.");
                return;
            }
            if (engine.trade(exchange::binance::constants::USDC_USDT, 
                            exchange::binance::Enums::OrderSide::BUY, 
                            exchange::binance::Enums::OrderType::MARKET, 
                            std::to_string(quantity),
                            0
                )) {
                    portfolio.sub_position(USDT, quantity * best_ask);
                    portfolio.add_position(USDC, quantity);
                    
                    usdc += quantity;
                    usdt -= quantity * best_ask;

                    LOG_INFO(logger::g_logger, "StrategyUSDCUSDTGrid::Buy USDT Order filled, current usdt: {}, current usdc: {}", usdt, usdc);
            }
            else {
                LOG_ERROR(logger::g_logger, "StrategyUSDCUSDTGrid::Order failed to fill");
            }
        }

        if (best_bid >= config_.sell_levels[0].price && usdc > 6) {
            const auto quantity = static_cast<int>(usdc);
            LOG_INFO(logger::g_logger, "StrategyUSDCUSDTGrid::bid level hit at {}, current usdt: {}, current usdc: {}", best_bid, usdt, usdc);
            if (engine.trade(exchange::binance::constants::USDC_USDT, 
                            exchange::binance::Enums::OrderSide::SELL, 
                            exchange::binance::Enums::OrderType::MARKET, 
                            std::to_string(quantity),
                            0
                )) {
                    portfolio.add_position(USDT, quantity * best_bid);
                    portfolio.sub_position(USDC, quantity);
                    usdt += quantity * best_bid;
                    usdc -= quantity;
                    LOG_INFO(logger::g_logger, "StrategyUSDCUSDTGrid::Sell USDC Order filled, current usdt: {}, current usdc: {}", usdt, usdc);
            }
            else {
                LOG_ERROR(logger::g_logger, "StrategyUSDCUSDTGrid::Order failed to fill");
            }
       }
    }

private:
    USDCUSDT_Grid_Config config_;
};


} // namespace arbitrage
} // namespace kitchen
} // namespace degen_crypto