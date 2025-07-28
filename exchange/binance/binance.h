#pragma once

#include <optional>
#include <memory>
#include <atomic>

#include "exchange/base_exchange.h"
#include "model.h"
#include "exchange/constants.h"
#include "common/utils/logger.h"
#include "common/utils/websocket.h"
#include "exchange/orderbook.h"
#include "kitchen/strategy_manager.h"

using namespace degen_crypto::logger;

namespace degen_crypto { namespace exchange { namespace binance {

class BinanceExchange : public ExchangeEngine<BinanceExchange> {

friend class ExchangeEngine<BinanceExchange>;
using order_book_t = order_book::OrderBook;
using strategy_manager_t = degen_crypto::kitchen::StrategyManager;
using instrument_balance_t = std::unordered_map<std::string, double>;
using exchange_onstart_cb = std::function<void(const instrument_balance_t&)>;

public:
    BinanceExchange() = default;

    ~BinanceExchange() {
        on_shutdown();
    }

    auto on_start(strategy_manager_t&, exchange_onstart_cb) -> bool;
    auto on_shutdown() -> bool;
    auto ping_exchange() -> bool;
    auto get_server_time() -> std::chrono::system_clock::time_point;
    inline auto exchange_name() const -> const std::string& {
        return constants::EXCHANGE_NAME;
    }
    inline auto exchange_host() const -> const std::string& {
        return exchange_hosts_[0];
    }
    inline auto ws_host() const -> const std::string& {
        return exchange_ws_host_;
    }
    inline auto ws_port() const -> const std::string& {
        return exchange_ws_port_;
    }

private:
    auto realtime_price(const std::string_view& symbol) -> double;
    auto trade_helper(const std::string& symbol, const std::string& side, const std::string& type, const std::string& quantity, double max_slippage) -> bool;
    auto mock_trade_helper(const std::string_view& symbol, const std::string_view& side, const std::string_view& type, const std::string_view& quantity, double max_slippage) -> bool;
    auto run() -> void;
    auto trade(const std::string& symbol, const std::string& side, const std::string& type, const std::string& quantity, double max_slippage) -> bool;
    auto symbol_balance(const std::string_view& symbol) -> double;
    auto account_balance() -> instrument_balance_t;
    auto depth_snapshot(const std::string_view& symbol) -> void;

private:
    auto start_websocket(const std::shared_ptr<order_book_t>& orderbook) -> void;
    auto subscribe_symbol(const std::string& symbol, const int level = 50) -> void;

public:
    std::unordered_map<std::string, std::shared_ptr<order_book_t>> order_books_;
};

} // namespace binance
} // namespace exchange
} // namespace degen_crypto
