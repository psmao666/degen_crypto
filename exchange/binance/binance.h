#pragma once

#include <optional>

#include "exchange/base_exchange.h"
#include "model.h"
#include "exchange/constants.h"

namespace degen_crypto { namespace exchange { namespace binance {

class BinanceExchange : public ExchangeEngine<BinanceExchange> {
    friend class ExchangeEngine<BinanceExchange>;
    
public:
    BinanceExchange() = default;
    ~BinanceExchange() {
        on_shutdown();
    }

    auto on_start() -> bool;
    auto on_shutdown() -> bool;
    auto ping_exchange() -> bool;
    auto get_server_time() -> std::chrono::system_clock::time_point;
    inline auto exchange_name() const -> const std::string& {
        return constants::EXCHANGE_NAME;
    }
    inline auto get_host() const -> const std::string& {
        return exchange_hosts_[0];
    }

};

} // namespace binance
} // namespace exchange
} // namespace degen_crypto
