#include "binance.h"
#include "model.h"
#include "common/utils/https.h"

#include <fstream>
#include <fmt/core.h>
#include <nlohmann/json.hpp>

namespace degen_crypto { namespace exchange { namespace binance {

auto BinanceExchange::on_start() -> bool {
    fmt::print("BinanceExchange::on_start()\n");
    std::ifstream config_file("exchange/binance/api_key.json");
    nlohmann::json config;
    config_file >> config;
    init_account_api_config(config["api_key"], config["secret_key"]);
    config_file.close();
    
    config_file.open("exchange/binance/config.json");
    config_file >> config;
    config_file.close();
    init_exchange_hosts(config["hosts"]);
    if (!ping_exchange()) {
        fmt::print("Binance bootstrap failed due to ping exchange failure\n");
        return false;
    }
    fmt::print("BinanceExchange::on_start() success\n");
    return true;
}  

auto BinanceExchange::on_shutdown() -> bool {
    fmt::print("BinanceExchange::on_shutdown() success\n");
    return true;
}

// We actually use /api/v3/time to check if the exchange is alive
auto BinanceExchange::ping_exchange() -> bool {
    return https_.get<ServerTimeResponse>(get_host(), "/api/v3/time").has_value();
}

auto BinanceExchange::get_server_time() -> std::chrono::system_clock::time_point {
    auto t = https_.get<ServerTimeResponse>(get_host(), "/api/v3/time");
    if (!t.has_value()) {
        throw std::runtime_error("Failed to get server time");
    }
    return t.value().get_time_point();
}


} // namespace binance
} // namespace exchange
} // namespace degen_crypto
