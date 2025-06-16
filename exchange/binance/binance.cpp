#include "binance.h"
#include "model.h"
#include "common/utils/https.h"

#include <fstream>
#include <fmt/core.h>
#include <nlohmann/json.hpp>

namespace degen_crypto { namespace exchange { namespace binance {

auto BinanceExchange::on_start() -> bool {
    fmt::print("BinanceExchange::on_start()\n");
    
    // Load API key config
    std::ifstream api_key_file(constants::EXCHANGE_API_KEY_FILE);
    if (!api_key_file.is_open()) {
        fmt::print("Failed to open {}\n", constants::EXCHANGE_API_KEY_FILE);
        return false;
    }
    
    nlohmann::json api_config;
    try {
        api_key_file >> api_config;
        if (!api_config.contains(constants::EXCHANGE_API_KEY_KEY) || !api_config.contains(constants::EXCHANGE_SECRET_KEY_KEY)) {
            fmt::print("{} missing required fields\n", constants::EXCHANGE_API_KEY_FILE);
            return false;
        }
        init_account_api_config(api_config[constants::EXCHANGE_API_KEY_KEY], api_config[constants::EXCHANGE_SECRET_KEY_KEY]);
    } catch (const nlohmann::json::exception& e) {
        fmt::print("Failed to parse {}: {}\n", constants::EXCHANGE_API_KEY_FILE, e.what());
        return false;
    }
    api_key_file.close();
    
    // Load exchange config
    std::ifstream config_file(constants::EXCHANGE_CONFIG_FILE);
    if (!config_file.is_open()) {
        fmt::print("Failed to open {}\n", constants::EXCHANGE_CONFIG_FILE);
        return false;
    }
    
    nlohmann::json config;
    try {
        config_file >> config;
        if (!config.contains(constants::EXCHANGE_HOSTS_KEY) || !config[constants::EXCHANGE_HOSTS_KEY].is_array() || config[constants::EXCHANGE_HOSTS_KEY].empty()) {
            fmt::print("{} missing or invalid hosts array\n", constants::EXCHANGE_CONFIG_FILE);
            return false;
        }
        init_exchange_hosts(config[constants::EXCHANGE_HOSTS_KEY]);
    } catch (const nlohmann::json::exception& e) {
        fmt::print("Failed to parse {}: {}\n", constants:: EXCHANGE_CONFIG_FILE, e.what());
        return false;
    }
    config_file.close();
    
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
    return https_.get<ServerTimeResponse>(get_host(), api::SERVER_TIME_API).has_value();
}

auto BinanceExchange::get_server_time() -> std::chrono::system_clock::time_point {
    auto t = https_.get<ServerTimeResponse>(get_host(), api::SERVER_TIME_API);
    if (!t.has_value()) {
        throw std::runtime_error("Failed to get server time");
    }
    return t.value().get_time_point();
}

} // namespace binance
} // namespace exchange
} // namespace degen_crypto
