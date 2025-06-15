#pragma once

#include <optional>
#include <vector>
#include <mutex>
#include "account.h"
#include "common/utils/https.h"

namespace degen_crypto { namespace exchange {

class ExchangeEngine {
public:
    ExchangeEngine() = default;
    virtual ~ExchangeEngine() = default;

protected:
    AccountConfig account_config_;
    std::vector<std::string> exchange_hosts_;
    common::utils::HttpsClient https_;
    
public:
    virtual auto on_start() -> bool = 0;
    virtual auto on_shutdown() -> bool = 0;
    virtual auto init_account_api_config(const std::string& api_key, const std::string& api_secret) -> void {
        this->account_config_.set_api_key(api_key);
        this->account_config_.set_api_secret(api_secret);
    }
    // given in the order of stability
    auto init_exchange_hosts(const std::vector<std::string>& exchange_hosts) -> void {
        this->exchange_hosts_ = exchange_hosts;
    }
    virtual auto ping_exchange() -> bool = 0;
};

} // namespace exchange
} // namespace degen_crypto
