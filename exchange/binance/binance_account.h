#pragma once

#include <chrono>
#include <thread>
#include <future>

#include "exchange/account.h"

namespace degen_crypto { namespace exchange { namespace binance {

class BinanceAccount final : public Account {
private:
    void refresh_handler() {

    }

public:
    virtual void init() override {
            
    }
};

} // namespace binance
} // namespace exchange
} // namespace degen_crypto