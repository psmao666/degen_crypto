#include "binance.h"
#include "model.h"
#include "common/utils/https.h"
#include "common/utils/signature.h"

#include <fstream>
#include <chrono>
#include <thread>
#include <fmt/core.h>
#include <nlohmann/json.hpp>

namespace degen_crypto { namespace exchange { namespace binance {

auto BinanceExchange::on_start() -> bool {
    LOG_INFO(g_logger, "BinanceExchange::on_start() triggered");
    
    // Load API key config
    std::ifstream api_key_file(constants::EXCHANGE_API_KEY_FILE);
    if (!api_key_file.is_open()) {
        LOG_ERROR(g_logger, "Failed to open {}", constants::EXCHANGE_API_KEY_FILE);
        return false;
    }
    
    nlohmann::json api_config;
    try {
        api_key_file >> api_config;
        if (!api_config.contains(constants::EXCHANGE_API_KEY_KEY) || !api_config.contains(constants::EXCHANGE_SECRET_KEY_KEY)) {
            LOG_ERROR(g_logger, "{} missing required fields", constants::EXCHANGE_API_KEY_FILE);
            return false;
        }
        init_account_api_config(api_config[constants::EXCHANGE_API_KEY_KEY], api_config[constants::EXCHANGE_SECRET_KEY_KEY]);
    } catch (const nlohmann::json::exception& e) {
        LOG_ERROR(g_logger, "Failed to parse {}: {}", constants::EXCHANGE_API_KEY_FILE, e.what());
        return false;
    }
    api_key_file.close();
    
    // Load exchange config
    std::ifstream config_file(constants::EXCHANGE_CONFIG_FILE);
    if (!config_file.is_open()) {
        LOG_ERROR(g_logger, "Failed to open {}", constants::EXCHANGE_CONFIG_FILE);
        return false;
    }
    
    nlohmann::json config;
    try {
        config_file >> config;
        if (!config.contains(constants::EXCHANGE_HOSTS_KEY) || !config[constants::EXCHANGE_HOSTS_KEY].is_array() || config[constants::EXCHANGE_HOSTS_KEY].empty()) {
            LOG_ERROR(g_logger, "{} missing or invalid hosts array", constants::EXCHANGE_CONFIG_FILE);
            return false;
        }
        init_exchange_hosts(config[constants::EXCHANGE_HOSTS_KEY]);
    } catch (const nlohmann::json::exception& e) {
        LOG_ERROR(g_logger, "Failed to parse {}: {}", constants::EXCHANGE_CONFIG_FILE, e.what());
        return false;
    }
    config_file.close();
    
    if (!ping_exchange()) {
        LOG_ERROR(g_logger, "Binance bootstrap failed due to ping exchange failure");
        return false;
    }
    
    LOG_INFO(g_logger, "BinanceExchange::on_start() success");
    run();
    
    return true;
}  

auto BinanceExchange::on_shutdown() -> bool {
    LOG_INFO(g_logger, "BinanceExchange::on_shutdown() success");
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

auto BinanceExchange::realtime_price(const std::string_view& symbol) -> double {
    const auto response = https_.get<nlohmann::json>(get_host(), fmt::format("{}?symbol={}", api::TICKER_PRICE_API, symbol));
    if (!response.has_value()) {
        throw std::runtime_error(fmt::format("Failed to get {} price", symbol));
    }
    return std::stod(response.value()["price"].get<std::string>());
}

// TODO: implement trade
auto BinanceExchange::trade(const std::string_view& symbol, const std::string_view& side, const std::string_view& type, const std::string_view& quantity, double max_slippage) -> bool {
    return false;
}

auto BinanceExchange::get_account_balance() -> double {
    auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();
    
    std::string query_string = fmt::format("timestamp={}", timestamp);
    std::string signature = common::utils::generate_signature(query_string, account_config_.api_secret());
    
    query_string += fmt::format("&signature={}", signature);
#ifdef DEBUG
    fmt::print("query_string: {}\n", query_string);
#endif
    const auto response = https_.get<nlohmann::json>(
        get_host(),
        fmt::format("{}?{}", api::ACCOUNT_API, query_string),
        {
            {"X-MBX-APIKEY", account_config_.api_key()}
        }
    );

    if (!response.has_value()) {
        throw std::runtime_error("Failed to get account balance");
    }
    
    double total_balance = 0.0;
    for (const auto& balance : response.value()["balances"]) {
        if (balance["asset"] == "USDT") {
            total_balance += std::stod(balance["free"].get<std::string>());
            total_balance += std::stod(balance["locked"].get<std::string>());
        }
    }
    
    return total_balance;
}

} // namespace binance
} // namespace exchange
} // namespace degen_crypto
