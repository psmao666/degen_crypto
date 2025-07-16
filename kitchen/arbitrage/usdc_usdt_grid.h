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
        LOG_INFO(logger::g_logger, "StrategyUSDCUSDTGrid::on_orderbook_update_callback() called");
    }

private:
    USDCUSDT_Grid_Config config_;
};


} // namespace arbitrage
} // namespace kitchen
} // namespace degen_crypto