#pragma once

namespace degen_crypto {
namespace lib {
namespace exchange {
namespace binance {

struct Constants {
    static constexpr auto EXCHANGE_NAME = "Binance";
    static constexpr auto SERVER_TIME_API = "/api/v3/time";
    static constexpr auto EXCHANGE_API_KEY_FILE = "exchange/binance/api_key.json";
    static constexpr auto EXCHANGE_CONFIG_FILE = "exchange/binance/config.json";
    static constexpr auto EXCHANGE_HOSTS_KEY = "hosts";
    static constexpr auto EXCHANGE_API_KEY_KEY = "api_key";
    static constexpr auto EXCHANGE_SECRET_KEY_KEY = "secret_key";
};
}
} // namespace exchange
} // namespace lib
} // namespace degen_crypto
