#pragma once

#include <string>
#include <vector>
#include <utility>
#include <memory>
#include <thread>
#include <atomic>
#include <functional>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include "common/utils/websocket.h"
#include "nlohmann/json_fwd.hpp"

namespace degen_crypto { namespace exchange { namespace order_book {

class OrderBookEntry {
public:
    OrderBookEntry() : price_(0.0), quantity_(0.0) {}
    OrderBookEntry(double price, double quantity) : price_(price), quantity_(quantity) {
    }
    ~OrderBookEntry() = default;

    OrderBookEntry(const OrderBookEntry& other) = default;
    OrderBookEntry(OrderBookEntry&& other) = default;
    auto operator=(const OrderBookEntry& other) -> OrderBookEntry& = default;
    auto operator=(OrderBookEntry&& other) -> OrderBookEntry& = default;

    inline auto price() const -> double { return price_; }
    inline auto quantity() const -> double { return quantity_; }

    friend void from_json(const nlohmann::json& j, OrderBookEntry& entry);

private:
    double price_;
    double quantity_;
};

class OrderBook {
public:
    using entry_t = OrderBookEntry;
    using orderbook_cb_t = std::function<void(const std::string& exchange_name, const std::string& symbol, const OrderBook& orderbook)>;

public:
    OrderBook(const std::string& symbol, const int level) : ws_ioc_(), ws_ctx_(boost::asio::ssl::context::tlsv12_client) {
        ws_ctx_.set_verify_mode(boost::asio::ssl::verify_none);
        ws_running_.store(true, std::memory_order_release);
        symbol_ = symbol;
        level_ = level;
    }
    ~OrderBook() {
        ws_running_.store(false, std::memory_order_relaxed);
    }

    inline auto symbol() const -> const std::string& { return symbol_; }
    inline auto level() const -> const int { return level_; }
    inline auto asks() -> std::vector<entry_t>& { return asks_; }
    inline auto bids() -> std::vector<entry_t>& { return bids_; }
    inline auto best_ask() const -> const entry_t& { return asks_.front(); }
    inline auto best_bid() const -> const entry_t& { return bids_.front(); }
    
    void set_orderbook_update_callback(orderbook_cb_t callback) {
        orderbook_update_callback_ = std::move(callback);
    }
    
    auto orderbook_message_handler(const std::string& message) -> void;
    
    // websocket related
    inline auto ws_ioc() -> boost::asio::io_context& { return ws_ioc_; }
    inline auto ws_ctx() -> boost::asio::ssl::context& { return ws_ctx_; }
    inline auto ws_client() -> std::shared_ptr<common::utils::WebSocketClient>& { return ws_client_; }
    inline auto ws_thread() -> std::jthread& { return ws_thread_; }
    inline auto ws_running() -> std::atomic<bool>& { return ws_running_; }

private:
    std::string symbol_;
    int level_;

    std::vector<entry_t> asks_;
    std::vector<entry_t> bids_;

    orderbook_cb_t orderbook_update_callback_;

private:
    boost::asio::io_context ws_ioc_;
    boost::asio::ssl::context ws_ctx_;
    std::shared_ptr<common::utils::WebSocketClient> ws_client_;
    std::jthread ws_thread_;
    std::atomic<bool> ws_running_;
};

} // namespace order_book
} // namespace exchange
} // namespace degen_crypto