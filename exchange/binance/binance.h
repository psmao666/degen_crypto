#pragma once

#include <optional>

#include "exchange/base_exchange.h"
#include "model.h"
#include "exchange/constants.h"
#include "common/utils/logger.h"

using namespace degen_crypto::logger;

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
private:
    auto realtime_price(const std::string_view& symbol) -> double;
    auto trade(const std::string_view& symbol, const std::string_view& side, const std::string_view& type, const std::string_view& quantity, double max_slippage) -> bool;
    auto get_account_balance() -> double;

    auto run() -> void {
        while (1) {
            fmt::print("BTC/USDT price: {}\n", realtime_price(constants::BTC_USDT));
            fmt::print("USDC/USDT price: {}\n", realtime_price(constants::USDC_USDT));
            fmt::print("Account balance: {}\n", get_account_balance());
            std::this_thread::sleep_for(std::chrono::microseconds(200));
        }
    }
};

} // namespace binance
} // namespace exchange
} // namespace degen_crypto
