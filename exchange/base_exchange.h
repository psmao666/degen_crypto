#pragma once

#include <optional>
#include <vector>
#include <mutex>
#include "account.h"
#include "common/utils/https.h"

namespace degen_crypto { namespace exchange {

template<typename Derived>
class ExchangeEngine {
public:
    ExchangeEngine() = default;
    ~ExchangeEngine() {
        derived().on_shutdown();
    }

protected:
    AccountConfig account_config_;
    std::vector<std::string> exchange_hosts_;
    common::utils::HttpsClient https_;

    // Helper method to access derived class methods
    auto derived() -> Derived& { return static_cast<Derived&>(*this); }
    auto derived() const -> const Derived& { return static_cast<const Derived&>(*this); }

// inherited methods
public:
    auto on_start() -> bool { return derived().on_start(); }
    auto on_shutdown() -> bool { return derived().on_shutdown(); }
    inline auto exchange_name() const -> const std::string& { return derived().exchange_name(); }
    inline auto get_host() const -> const std::string& { return derived().get_host(); }
    auto get_server_time() -> std::chrono::system_clock::time_point { return derived().get_server_time(); }

// base impl    
public:
    inline auto init_account_api_config(const std::string& api_key, const std::string& api_secret) -> void {
        this->account_config_.set_api_key(api_key);
        this->account_config_.set_api_secret(api_secret);
    }
    // given in the order of stability
    inline auto init_exchange_hosts(const std::vector<std::string>& exchange_hosts) -> void {
        this->exchange_hosts_ = exchange_hosts;
    }
    auto ping_exchange() -> bool { return derived().ping_exchange(); }
};

} // namespace exchange
} // namespace degen_crypto
