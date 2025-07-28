#pragma once

#include <vector>
#include <unordered_map>

#include "account.h"

namespace degen_crypto { namespace exchange {

class PositionManager {
public:
    // mapping exchange -> account
    using portfolio_t = std::unordered_map<std::string, Account>;
    using instrument_balance_t = std::unordered_map<std::string, double>;

public:
    PositionManager() = default;
    ~PositionManager() = default;

    void add_exchange(const std::string& exchange_name, const instrument_balance_t& balances) {
        portfolios[exchange_name].init(exchange_name, balances);
    }

    portfolio_t& portfolio() { return portfolios; }

private:
    portfolio_t portfolios;     
};

} // namespace exchange
} // namespace degen_crypto