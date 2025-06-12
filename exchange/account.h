#pragma once

#include <fmt/core.h>
#include <string>

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
private:
    std::string api_key_;
    std::string api_secret_;
};

} // namespace exchange
} // namespace degen_crypto