#pragma once

#include <optional>
#include "../base_exchange.h"

namespace degen_crypto { namespace exchange { namespace binance {

class BinanceExchange : public ExchangeEngine {
public:
    auto on_start() -> void override;
    auto on_shutdown() -> void override;
};

} // namespace binance
} // namespace exchange
} // namespace degen
