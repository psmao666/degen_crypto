#pragma once

#include "../../kitchen/strategy_manager.h"
#include "../../kitchen/strategy_config.h"
#include "nlohmann/json.hpp"

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

       if (orderbook.best_bid().price() <= config_.buy_levels[0].price && usdt_ > 0) {
        usdc_ += usdt_ * config_.buy_levels[0].buy_capital_ratio / orderbook.best_bid().price();
        usdt_ -= usdt_ * config_.buy_levels[0].buy_capital_ratio;
    
        LOG_INFO(logger::g_logger, "StrategyUSDCUSDTGrid::bid level hit at {}, current usdt: {}, current usdc: {}", orderbook.best_bid().price(), usdt_, usdc_);
        
        // buy
       }
       if (orderbook.best_ask().price() >= config_.sell_levels[0].price && usdc_ > 0) {
        
        usdt_ += usdc_ * orderbook.best_ask().price();
        usdc_ = 0;
        LOG_INFO(logger::g_logger, "StrategyUSDCUSDTGrid::ask level hit at {}, current usdt: {}, current usdc: {}", orderbook.best_ask().price(), usdt_, usdc_);
        // sell
       }
    }

private:
    double usdt_ = 1000;
    double usdc_ = 0;
    USDCUSDT_Grid_Config config_;
};


} // namespace arbitrage
} // namespace kitchen
} // namespace degen_crypto