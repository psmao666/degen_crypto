#include "exchange/orderbook.h"
#include "common/utils/logger.h"
#include "nlohmann/json.hpp"
#include "exchange/constants.h"

namespace degen_crypto { namespace exchange { namespace order_book {

void from_json(const nlohmann::json& j, OrderBookEntry& entry) {
    entry.price_ = std::stod(j.at(0).get<std::string>());
    entry.quantity_ = std::stod(j.at(1).get<std::string>());
}

auto OrderBook::orderbook_message_handler(const std::string& message) -> void {
    try {
        nlohmann::json json_message = nlohmann::json::parse(message);

        asks_ = json_message.at("asks").get<std::vector<entry_t>>();
        bids_ = json_message.at("bids").get<std::vector<entry_t>>();
        
        if (!asks_.empty() && !bids_.empty()) {
            if (orderbook_update_callback_) {
                orderbook_update_callback_(binance::constants::EXCHANGE_NAME, this->symbol(), *this);
            }
        }

    } catch (const nlohmann::json::exception& e) {
        LOG_ERROR(logger::g_logger, "Error parsing orderbook JSON for {}: {}", symbol_, e.what());
    } catch (const std::exception& e) {
        LOG_ERROR(logger::g_logger, "Error processing orderbook message for {}: {}", symbol_, e.what());
    }
}

} // namespace order_book
} // namespace exchange
} // namespace degen_crypto