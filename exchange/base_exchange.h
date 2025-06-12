#pragma once

#include <optional>

#include "account.h"

namespace degen_crypto { namespace exchange {

class ExchangeEngine {
private:
    AccountConfig account_config_;
public:
    virtual auto on_start() -> void = 0;
    virtual auto on_shutdown() -> void = 0;
};

} // namespace exchange
} // namespace degen
