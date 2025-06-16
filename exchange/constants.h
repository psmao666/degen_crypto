#pragma once

#include <string>

namespace degen_crypto { namespace exchange { 

namespace binance {

    namespace api {
        static const std::string SERVER_TIME_API = "/api/v3/time";
        static const std::string TICKER_PRICE_API = "/api/v3/ticker/price";
        static const std::string ACCOUNT_API = "/api/v3/account";
    } // namespace api

    namespace constants {
        const std::string EXCHANGE_NAME = "Binance";

        // exchange config related constants
        const std::string EXCHANGE_API_KEY_FILE = "exchange/binance/api_key.json";
        const std::string EXCHANGE_CONFIG_FILE = "exchange/binance/config.json";
        const std::string EXCHANGE_HOSTS_KEY = "hosts";
        const std::string EXCHANGE_API_KEY_KEY = "api_key";
        const std::string EXCHANGE_SECRET_KEY_KEY = "secret_key";

        // currency pair
        const std::string BTC_USDT = "BTCUSDT";
        const std::string USDC_USDT = "USDCUSDT";
    } // namespace constants

} // namespace binance


} // namespace exchange
} // namespace degen_crypto