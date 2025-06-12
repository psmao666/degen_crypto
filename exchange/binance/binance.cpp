#include "binance.h"
#include <fstream>
#include <fmt/core.h>
#include <nlohmann/json.hpp>

namespace degen_crypto { namespace exchange { namespace binance {

auto BinanceExchange::on_start() -> void {
    fmt::print("BinanceExchange::on_start()\n");
    std::ifstream config_file("exchange/binance/config.json");
    nlohmann::json config;
    config_file >> config;
    fmt::print("BinanceExchange::on_start() config: {}\n", config.dump());
}

auto BinanceExchange::on_shutdown() -> void {
    fmt::print("BinanceExchange::on_shutdown()\n");
}

} // namespace binance
} // namespace exchange
} // namespace degen
