#pragma once

#include <string>

namespace degen_crypto { namespace exchange { 

namespace binance {

    namespace api {
        static const std::string SERVER_TIME_API = "/api/v3/time";
        static const std::string TICKER_PRICE_API = "/api/v3/ticker/price";
        static const std::string ACCOUNT_API = "/api/v3/account";
        static const std::string ORDER_API = "/api/v3/order";
    } // namespace api

    namespace constants {
        const std::string EXCHANGE_NAME = "BINANCE";

        // exchange config related constants
        const std::string EXCHANGE_CONFIG_FILE = "exchange/binance/config.json";
        const std::string EXCHANGE_HOSTS_KEY = "hosts";
        const std::string EXCHANGE_WS_KEY = "ws";
        const std::string EXCHANGE_WS_HOST_KEY = "host";
        const std::string EXCHANGE_WS_PORT_KEY = "port";
        const std::string EXCHANGE_API_KEY = "API_KEY";
        const std::string EXCHANGE_SECRET_KEY = "SECRET_KEY";
        const std::string EXCHANGE_SUBSCRIPTIONS_KEY = "subscriptions";
        const std::string EXCHANGE_SUBSCRIPTION_SYMBOL_KEY = "symbol";
        const std::string EXCHANGE_SUBSCRIPTION_LEVEL_KEY = "level";

        // currency pair
        const std::string BTC_USDT = "BTCUSDT";
        const std::string USDC_USDT = "USDCUSDT";


        // currency
        const std::string USDT = "USDT";
        const std::string USDC = "USDC";
        const std::string BTC = "BTC";
        const std::string ETH = "ETH";
        const std::string SOL = "SOL";
        const std::string XRP = "XRP";
        const std::string ADA = "ADA";
        const std::string DOT = "DOT";
        const std::string LINK = "LINK";
        const std::string BNB = "BNB";
        const std::string XLM = "XLM";
        const std::string LTC = "LTC";  
        const std::string BCH = "BCH";
        const std::string XMR = "XMR";
        const std::string FDUSD = "FDUSD";
    } // namespace constants

} // namespace binance


} // namespace exchange
} // namespace degen_crypto