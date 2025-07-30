#include "strategy_manager.h"
#include "kitchen/arbitrage/fdusd_usdt_grid.h"
#include "kitchen/strategy_config.h"
#include "common/utils/logger.h"

#include <fstream>
// strategies
#include "kitchen/arbitrage/usdc_usdt_grid.h"

using namespace degen_crypto::logger;

namespace degen_crypto { namespace kitchen {

const char* strategy_config_file_path = "kitchen/strategy_configs.json";

StrategyManager::StrategyManager() {
    std::ifstream strategy_config_file(strategy_config_file_path);
    
    if (!strategy_config_file.is_open()) {
        LOG_ERROR(g_logger, "Failed to open strategy config file");
        throw std::runtime_error("Failed to open strategy config file");
    }
    nlohmann::json strategy_config;
    try {
        strategy_config_file >> strategy_config;
        hook<arbitrage::StrategyUSDCUSDTGrid>(strategy_config["USDC-USDT_Grid"]);
        hook<arbitrage::StrategyFDUSDUSDTGrid>(strategy_config["FDUSD-USDT_Grid"]);
    } catch (const std::exception& e) {
        LOG_ERROR(g_logger, "Failed to parse strategy config: {}", e.what());
        throw std::runtime_error("Failed to parse strategy config");
    }
}

} // namespace kitchen
} // namespace degen_crypto