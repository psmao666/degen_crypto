#pragma once

#include "../../kitchen/strategy_manager.h"
#include "../../kitchen/strategy_config.h"
#include "common/utils/logger.h"
#include "nlohmann/json.hpp"
#include "../../exchange/constants.h"
#include "quill/LogMacros.h"
#include <string>

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
        auto& portfolio = position_manager().portfolio()[exchange_name];
        constexpr auto& USDC = exchange::binance::constants::USDC;
        constexpr auto& USDT = exchange::binance::constants::USDT;
        auto usdc = portfolio.get_position(USDC);
        auto usdt = portfolio.get_position(USDT);
        auto& engine = exchange_engine();
        const auto best_bid = orderbook.best_bid().price();
        const auto best_ask = orderbook.best_ask().price();
        LOG_INFO(logger::g_logger, "StrategyUSDCUSDTGrid::Current portfolio: [usdc: {}, usdt: {}], best bid: {}, best ask: {}", usdc, usdt, best_bid, best_ask);

        if (best_ask <= config_.buy_levels[0].price && usdt > 0) {
            LOG_INFO(logger::g_logger, "StrategyUSDCUSDTGrid::ask level hit at {}, current usdt: {}, current usdc: {}", best_ask, usdt, usdc);
            const auto quantity = std::round(usdt * config_.buy_levels[0].buy_capital_ratio / best_ask);
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
                    portfolio.sub_position(USDT, usdt * config_.buy_levels[0].buy_capital_ratio);
                    portfolio.add_position(USDC, usdt * config_.buy_levels[0].buy_capital_ratio / best_ask);
                    
                    usdc += usdt * config_.buy_levels[0].buy_capital_ratio / best_ask;
                    usdt -= usdt * config_.buy_levels[0].buy_capital_ratio;

                    LOG_INFO(logger::g_logger, "StrategyUSDCUSDTGrid::Buy USDT Order filled, current usdt: {}, current usdc: {}", usdt, usdc);
            }
            else {
                LOG_ERROR(logger::g_logger, "StrategyUSDCUSDTGrid::Order failed to fill");
            }
        }

        if (best_bid >= config_.sell_levels[0].price && usdc > 6) {
            const auto quantity = std::round(usdc);
            LOG_INFO(logger::g_logger, "StrategyUSDCUSDTGrid::bid level hit at {}, current usdt: {}, current usdc: {}", best_bid, usdt, usdc);
            if (engine.trade(exchange::binance::constants::USDC_USDT, 
                            exchange::binance::Enums::OrderSide::SELL, 
                            exchange::binance::Enums::OrderType::MARKET, 
                            std::to_string(quantity),
                            0
                )) {
                    portfolio.add_position(USDT, usdc * best_bid);
                    portfolio.sub_position(USDC, usdc);
                    usdt += usdc * best_bid;
                    usdc = 0;
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