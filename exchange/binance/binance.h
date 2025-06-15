#pragma once

#include <optional>
#include "exchange/base_exchange.h"

namespace degen_crypto { namespace exchange { namespace binance {

class BinanceExchange final : public ExchangeEngine {
public:
    BinanceExchange() = default;
    ~BinanceExchange() override {
        on_shutdown();
    }
public:
    auto on_start() -> bool override;
    auto on_shutdown() -> bool override;
    auto ping_exchange() -> bool override;
private:
    // TODO: right now we just pick the first, but in the future we will maintain usability
    auto get_host() const -> const std::string & {
        return exchange_hosts_[0];
    }
    auto get_server_time() -> std::chrono::system_clock::time_point;
};

} // namespace binance
} // namespace exchange
} // namespace degen
