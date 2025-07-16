#include "binance.h"
#include "model.h"
#include "common/utils/https.h"
#include "kitchen/strategy_manager.h"

#include <fstream>
#include <chrono>
#include <thread>
#include <fmt/core.h>
#include <nlohmann/json.hpp>
#include <algorithm>
#include <cctype>

namespace degen_crypto { namespace exchange { namespace binance {

auto BinanceExchange::on_start(strategy_manager_t& strategy_manager) -> bool {
    LOG_INFO(g_logger, "{}::on_start() triggered", exchange_name());
    
    // Load API key from environment variables
    const char* api_key = std::getenv((constants::EXCHANGE_NAME + "_" + constants::EXCHANGE_API_KEY).c_str());
    const char* secret_key = std::getenv((constants::EXCHANGE_NAME + "_" + constants::EXCHANGE_SECRET_KEY).c_str());
    
    if (!api_key || !secret_key) {
        LOG_ERROR(g_logger, "Missing required environment variables: {} and/or {}", 
            constants::EXCHANGE_NAME + "_" + constants::EXCHANGE_API_KEY, 
            constants::EXCHANGE_NAME + "_" + constants::EXCHANGE_SECRET_KEY);
        return false;
    }
    init_account_api_config(api_key, secret_key);
    LOG_INFO(g_logger, "{}::on_start() account api config loaded successfully", exchange_name());

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
            throw std::runtime_error("Failed to load hosts");
        }

        init_exchange_hosts(config[constants::EXCHANGE_HOSTS_KEY]);

        // Load ws host
        if (!config.contains(constants::EXCHANGE_WS_KEY)) {
            LOG_ERROR(g_logger, "{} missing or invalid ws_host", constants::EXCHANGE_CONFIG_FILE);
            throw std::runtime_error("Failed to load ws host");
        }

        exchange_ws_host_ = config[constants::EXCHANGE_WS_KEY][constants::EXCHANGE_WS_HOST_KEY];
        exchange_ws_port_ = config[constants::EXCHANGE_WS_KEY][constants::EXCHANGE_WS_PORT_KEY];
  
        // Load subscriptions
        if (config.contains(constants::EXCHANGE_SUBSCRIPTIONS_KEY) && config[constants::EXCHANGE_SUBSCRIPTIONS_KEY].is_array()) {
            for (const auto& subscription : config[constants::EXCHANGE_SUBSCRIPTIONS_KEY]) {
                subscribe_symbol(subscription[constants::EXCHANGE_SUBSCRIPTION_SYMBOL_KEY], subscription[constants::EXCHANGE_SUBSCRIPTION_LEVEL_KEY]);
            }
        } else {
            LOG_ERROR(g_logger, "{} missing or invalid subscriptions array", constants::EXCHANGE_CONFIG_FILE);
            throw std::runtime_error("Failed to load subscriptions");
        }   
    } catch (const nlohmann::json::exception& e) {
        LOG_ERROR(g_logger, "Failed to parse {}: {}", constants::EXCHANGE_CONFIG_FILE, e.what());
        config_file.close();
        return false;
    }
    LOG_INFO(g_logger, "{}::on_start() exchange config loaded successfully", exchange_name());

    if (!ping_exchange()) {
        LOG_ERROR(g_logger, "{} bootstrap failed due to ping exchange failure", exchange_name());
        return false;
    }
    

    LOG_INFO(g_logger, "{}::loading strategy manager callbacks...", exchange_name());
    // Set up orderbook callbacks to notify strategies
    for (auto& [symbol, orderbook] : order_books_) {
        orderbook->set_orderbook_update_callback(
            [&strategy_manager](const std::string& exchange_name, const std::string& symbol, const order_book::OrderBook& orderbook) {
                strategy_manager.notify_orderbook_update(exchange_name, symbol, orderbook);
            }
        );
    }
    LOG_INFO(g_logger, "{}::on_start() success, running now...", exchange_name());
    
    run();
    
    return true;
}  

auto BinanceExchange::on_shutdown() -> bool {
    for (auto& [symbol, orderbook] : order_books_) {
        orderbook->ws_running().store(false);
    }
    
    LOG_INFO(g_logger, "{}::on_shutdown() success", exchange_name());
    return true;
}

// We actually use /api/v3/time to check if the exchange is alive
auto BinanceExchange::ping_exchange() -> bool {
    return https_.get<ServerTimeResponse>(exchange_host(), api::SERVER_TIME_API, account_config_.api_secret()).has_value();
}

auto BinanceExchange::get_server_time() -> std::chrono::system_clock::time_point {
    auto t = https_.get<ServerTimeResponse>(exchange_host(), api::SERVER_TIME_API, account_config_.api_secret());
    if (!t.has_value()) {
        throw std::runtime_error("Failed to get server time");
    }
    return t.value().get_time_point();
}

auto BinanceExchange::realtime_price(const std::string_view& symbol) -> double {
    const auto response = https_.get<nlohmann::json>(exchange_host(), fmt::format("{}?symbol={}", api::TICKER_PRICE_API, symbol), account_config_.api_secret());
    if (!response.has_value()) {
        throw std::runtime_error(fmt::format("Failed to get {} price", symbol));
    }
    return std::stod(response.value()["price"].get<std::string>());
}

auto BinanceExchange::depth_snapshot(const std::string_view& symbol) -> void {
    const auto response = https_.get<nlohmann::json>(exchange_host(), fmt::format("{}?symbol={}&limit=5000", api::DEPTH_SNAPSHOT_API, symbol), account_config_.api_secret());
    if (!response.has_value()) {
        throw std::runtime_error(fmt::format("Failed to get {} depth snapshot", symbol));
    }
    
}

auto BinanceExchange::trade(const std::string_view& symbol, const std::string_view& side, const std::string_view& type, const std::string_view& quantity, double max_slippage) -> bool {
#ifdef MOCK_TRADE
    return mock_trade_helper(symbol, side, type, quantity, max_slippage);
#else
    return trade_helper(symbol, side, type, quantity, max_slippage);
#endif
}

auto BinanceExchange::trade_helper(const std::string_view& symbol, const std::string_view& side, const std::string_view& type, const std::string_view& quantity, double max_slippage) -> bool {
    const auto response = https_.get<nlohmann::json>(
        exchange_host(),
        fmt::format("{}?symbol={}&side={}&type={}&quantity={}", api::ORDER_API, symbol, side, type, quantity),
        account_config_.api_secret(),
        {
            {"X-MBX-APIKEY", account_config_.api_key()}
        },
        true
    );

    if (!response.has_value()) {
        LOG_ERROR(g_logger, "Failed to place order for {} {} {}", symbol, side, quantity);
        return false;
    }

    LOG_INFO(g_logger, "Order placed successfully: {} {} {}", symbol, side, quantity);
    return true;
}

auto BinanceExchange::mock_trade_helper(const std::string_view& symbol, const std::string_view& side, const std::string_view& type, const std::string_view& quantity, double max_slippage) -> bool {
    LOG_INFO(g_logger, "Mock trade placed successfully: {} {} {}", symbol, side, quantity);
    return true;
}

auto BinanceExchange::get_account_balance(const std::string_view& symbol) -> double {
    const auto response = https_.get<nlohmann::json>(
        exchange_host(),
        api::ACCOUNT_API,
        account_config_.api_secret(),
        {
            {"X-MBX-APIKEY", account_config_.api_key()}
        },
        true
    );

    if (!response.has_value()) {
        throw std::runtime_error("Failed to get account balance");
    }
    
    double total_balance = 0.0;
    for (const auto& balance : response.value()["balances"]) {
        if (balance["asset"] == symbol) {
            total_balance += std::stod(balance["free"].get<std::string>());
            total_balance += std::stod(balance["locked"].get<std::string>());
        }
    }
    
    return total_balance;
}

auto BinanceExchange::subscribe_symbol(const std::string& symbol, const int level) -> void {
    auto orderbook = std::make_shared<order_book_t>(symbol, level);
    order_books_[symbol] = orderbook;
    start_websocket(orderbook);
}

auto BinanceExchange::start_websocket(const std::shared_ptr<order_book_t>& orderbook) -> void {
    orderbook->ws_client() = std::make_shared<common::utils::WebSocketClient>(orderbook->ws_ioc(), orderbook->ws_ctx());
    
    orderbook->ws_thread() = std::jthread([this, orderbook]() {
        LOG_INFO(g_logger, "{}::subscribing {} with level of {}", exchange_name(), orderbook->symbol(), orderbook->level());

        orderbook->ws_client()->connect(
            ws_host(),
            ws_port(),
            fmt::format("/ws/{}@depth{}", orderbook->symbol(), orderbook->level()),
            [this, orderbook](const std::string& msg) { orderbook->orderbook_message_handler(msg); }
        );
        
        orderbook->ws_ioc().run();
    });
}

auto BinanceExchange::run() -> void {
    while (1) {
        
    }
}

} // namespace binance
} // namespace exchange
} // namespace degen_crypto
