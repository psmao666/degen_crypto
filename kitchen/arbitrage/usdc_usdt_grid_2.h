#pragma once

#include "../../kitchen/strategy_manager.h"
#include "../../kitchen/strategy_config.h"
#include "nlohmann/json.hpp"
#include "../../exchange/constants.h"
#include "quill/LogMacros.h"

#include <unordered_set>
#include <common/utils/logger.h>
#include <common/utils/compare_case_insensitive.h>


namespace degen_crypto { namespace kitchen { namespace arbitrage {

class StrategyUSDCUSDTGrid_2 : public Strategy<StrategyUSDCUSDTGrid_2> {
private:
    static constexpr int multiplier = 10'000;

public:
    StrategyUSDCUSDTGrid_2() = default;
    ~StrategyUSDCUSDTGrid_2() = default;

    bool on_start() { run(); return true; }
    bool on_shutdown() { return true; }
    bool load_params(const nlohmann::json& params) { 
        return config_.load_params(params);
    }

    const char* strategy_name() const { return "USDC-USDT_Grid_2"; }
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
        
        // check for potential buys
        if (best_ask <= config_.buy_levels[0].price && usdt > 0) {
            const auto ask_price = static_cast<int>(best_ask * multiplier);
            if (!positions_[ask_price]) {
                const auto quantity = static_cast<int>(usdt * config_.buy_levels[0].buy_capital_ratio / best_ask);
                if (quantity <= 6) {
                    LOG_INFO(logger::g_logger, "{}::Not enough notional, not buying.", strategy_name());
                } else {
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
                        positions_[ask_price] += quantity;
                        LOG_INFO(logger::g_logger, "{}::Buy USDT Order filled, bought {}, current usdt: {}, current usdc: {}", strategy_name(), quantity, usdt, usdc);
                    }
                    else {
                        LOG_ERROR(logger::g_logger, "{}:Order failed to fill", strategy_name());
                    }
                }
            }
        }

        const auto bid_price = static_cast<int>(best_bid * multiplier);
        // sell all positions below current bid price
        for (int p = bid_price - 1; p >= 9900; -- p) {
            
            if (!positions_[p]) continue;
            const auto quantity = static_cast<int>(positions_[p]);
            LOG_INFO(logger::g_logger, "{}::bid level hit at {}, current usdt: {}, current usdc: {}", strategy_name(), p * 1.0 / multiplier, usdt, usdc);
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
                    positions_[p] = 0;
                    LOG_INFO(logger::g_logger, "{}::Sell USDC Order filled, sold {}, current usdt: {}, current usdc: {}", strategy_name(), quantity, usdt, usdc);
            }
            else {
                LOG_ERROR(logger::g_logger, "{}::Order failed to fill", strategy_name());
            }
        }
    }

private:
    USDCUSDT_Grid_2_Config config_;
    std::array<double, 20000> positions_;
};


} // namespace arbitrage
} // namespace kitchen
} // namespace degen_crypto