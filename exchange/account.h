#pragma once

#include "quill/LogMacros.h"
#include "common/utils/logger.h"

#include <fmt/core.h>
#include <ostream>
#include <string>
#include <unordered_map>
#include <functional>
#include <mutex>


using namespace degen_crypto::logger;

namespace degen_crypto { namespace exchange {

class AccountConfig {
public:
    AccountConfig() = default;
    AccountConfig(const std::string& api_key, const std::string& api_secret) : api_key_(api_key), api_secret_(api_secret) {}
    void set_api_key(const std::string& api_key) {
        api_key_ = api_key;
    }
    void set_api_secret(const std::string& api_secret) {
        api_secret_ = api_secret;
    }
    const std::string& api_key() const { return api_key_; }
    const std::string& api_secret() const { return api_secret_; }
private:
    std::string api_key_;
    std::string api_secret_;
};

class Position {
public:
    using quantity_t = double;
    using price_t = double;

public:
    inline void set_holding_amount(const quantity_t& amount) { holding_amount_ = amount; }
    inline quantity_t& holding_amount() { return holding_amount_; }
    inline const quantity_t& holding_amount() const { return holding_amount_; }
    
    inline void add(const quantity_t& amount) noexcept { holding_amount_ += amount; }
    inline void sub(const quantity_t& amount) noexcept { holding_amount_ -= amount; }

private:
    quantity_t holding_amount_{0};
};

class Account {
public:
    using symbol_t = std::string;
    using positions_t = std::unordered_map<symbol_t, Position>;
    using account_config_t = AccountConfig;
    using quantity_t = Position::quantity_t;
    using account_refresh_handler = std::function<void(Account&)>;
    using instrument_balance_t = std::unordered_map<std::string, double>;

public:
    Account() = default;
    ~Account() = default;

    inline void add_position(const symbol_t& symbol, const quantity_t& amount) noexcept {
        std::lock_guard<std::mutex> lock(mutex_);
        positions_[symbol].add(amount);
    }

    inline void sub_position(const symbol_t& symbol, const quantity_t& amount) noexcept {
        std::lock_guard<std::mutex> lock(mutex_);
        positions_[symbol].sub(amount);
    }

public:

    void init(const instrument_balance_t& balances) {
        LOG_INFO(g_logger, "on initing balance");
        for (const auto& [symbol, balance] : balances) {
            if (balance > 0) {
                LOG_INFO(g_logger, "{}:{}", symbol, balance);
                add_position(symbol, balance);
            }
        }
    }

    inline auto positions() const -> const positions_t& { return this->positions_; }

private:
    account_config_t account_config_;
    positions_t positions_;
    std::mutex mutex_;
};

} // namespace exchange
} // namespace degen_crypto