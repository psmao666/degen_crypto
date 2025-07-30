#pragma once

#include "../../kitchen/strategy_manager.h"
#include "../../kitchen/strategy_config.h"
#include "nlohmann/json.hpp"
#include "../../exchange/constants.h"
#include "quill/LogMacros.h"

#include <common/utils/logger.h>
#include <common/utils/compare_case_insensitive.h>

namespace degen_crypto { namespace kitchen { namespace arbitrage {

class StrategyFDUSDUSDTGrid final : public Strategy<StrategyFDUSDUSDTGrid> {
public:
    StrategyFDUSDUSDTGrid() = default;
    ~StrategyFDUSDUSDTGrid() = default;

    bool on_start() override { run(); return true; }
    bool on_shutdown() override { return true; }
    bool load_params(const nlohmann::json& params) override { 
        return config_.load_params(params);
    }

    const char* strategy_name() const override { return "FDUSD_USDT_Grid"; }
    void run() override { }

    virtual void on_orderbook_update_callback(const std::string& exchange_name, const std::string& symbol, const order_book_t& orderbook) override {
        if (!common::utils::compare_str_insensitve(symbol, exchange::binance::constants::FDUSD_USDT)) return;

        auto& portfolio = position_manager().portfolio()[exchange_name];
        constexpr auto& FDUSD = exchange::binance::constants::FDUSD;
        constexpr auto& USDT = exchange::binance::constants::USDT;
        auto fdusd = portfolio.get_position(FDUSD);
        auto usdt = portfolio.get_position(USDT);
        auto& engine = exchange_engine();
        const auto best_bid = orderbook.best_bid().price();
        const auto best_ask = orderbook.best_ask().price();
        // LOG_INFO(logger::g_logger, "{}::current best bid at {}, current best ask at {}, target at {}, usdt: {}, fdusd: {}", strategy_name(), best_bid, best_ask, config_.buy_levels[0].price, usdt, fdusd);

        if (best_ask <= config_.buy_levels[0].price && usdt > 5) {
            const auto quantity = static_cast<int>(usdt * config_.buy_levels[0].buy_capital_ratio / best_ask);
            if (quantity <= 6) {
                LOG_INFO(logger::g_logger, "{}::Not enough notional, not buying.", strategy_name());
                return;
            }
            if (engine.trade(exchange::binance::constants::FDUSD_USDT, 
                            exchange::binance::Enums::OrderSide::BUY, 
                            exchange::binance::Enums::OrderType::MARKET, 
                            std::to_string(quantity),
                            0
                )) {
                    portfolio.sub_position(USDT, quantity * best_ask);
                    portfolio.add_position(FDUSD, quantity);
                    
                    fdusd += quantity;
                    usdt -= quantity * best_ask;

                    LOG_INFO(logger::g_logger, "{}::Buy USDT Order filled, current usdt: {}, current fdusd: {}", strategy_name(), usdt, fdusd);
            }
            else {
                LOG_ERROR(logger::g_logger, "{}::Order failed to fill", strategy_name());
            }
        }

        if (best_bid >= config_.sell_levels[0].price && fdusd > 6) {
            const auto quantity = static_cast<int>(fdusd);
            LOG_INFO(logger::g_logger, "{}::bid level hit at {}, current usdt: {}, current usdc: {}", strategy_name(), best_bid, usdt, fdusd);
            if (engine.trade(exchange::binance::constants::USDC_USDT, 
                            exchange::binance::Enums::OrderSide::SELL, 
                            exchange::binance::Enums::OrderType::MARKET, 
                            std::to_string(quantity),
                            0
                )) {
                    portfolio.add_position(USDT, quantity * best_bid);
                    portfolio.sub_position(FDUSD, quantity);
                    usdt += quantity * best_bid;
                    fdusd -= quantity;
                    LOG_INFO(logger::g_logger, "{}::Sell USDC Order filled, current usdt: {}, current usdc: {}", strategy_name(), usdt, fdusd);
            }
            else {
                LOG_ERROR(logger::g_logger, "{}::Order failed to fill", strategy_name());
            }
       }
    }

private:
    USDCUSDT_Grid_Config config_;
};


} // namespace arbitrage
} // namespace kitchen
} // namespace degen_crypto