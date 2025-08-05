#pragma once

#include <string>

namespace degen_crypto { namespace exchange { 

namespace binance {

    namespace api {
        static const std::string SERVER_TIME_API = "/api/v3/time";
        static const std::string TICKER_PRICE_API = "/api/v3/ticker/price";
        static const std::string ACCOUNT_API = "/api/v3/account";
        static const std::string ORDER_API = "/api/v3/order";
        static const std::string DEPTH_SNAPSHOT_API = "/api/v3/depth";
    } // namespace api
    
    namespace Enums {
        namespace OrderType {
            static const std::string LIMIT = "LIMIT";
            static const std::string MARKET = "MARKET";
            static const std::string STOP_LOSS = "STOP_LOSS";
            static const std::string STOP_LOSS_LIMIT = "STOP_LOSS_LIMIT";
            static const std::string TAKE_PROFIT = "TAKE_PROFIT";
            static const std::string TAKE_PROFIT_LIMIT = "TAKE_PROFIT_LIMIT";
            static const std::string LIMIT_MAKER = "LIMIT_MAKER";
        }; // namespace OrderType

        namespace OrderSide {
            static const std::string BUY = "BUY";
            static const std::string SELL = "SELL";
        }; // namespace OrderSide

        namespace TimeInForce {
            static const std::string GTC = "GTC";
            static const std::string IOC = "IOC";
            static const std::string FOK = "FOK";
        };
    } // namespace enums

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
        
        const std::string STRATEGY_USDC_USDT_GRID2_POS_FILE = "kitchen/arbitrage/usdc_usdt_grid_2.pos";

        // currency pair
        const std::string BTC_USDT = "BTCUSDT";
        const std::string USDC_USDT = "USDCUSDT";
        const std::string FDUSD_USDT = "FDUSDUSDT";
        const std::string USD1_USDT = "USD1USDT";

        // currency
        const std::string USDT = "USDT";
        const std::string USDC = "USDC";
        const std::string USD1 = "USD1";
        const std::string FDUSD = "FDUSD";
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
    } // namespace constants

} // namespace binance

namespace bybit {

    namespace constants {
        const std::string EXCHANGE_NAME = "BYBIT";
        
    }
} // namespace bybit

} // namespace exchange
} // namespace degen_crypto