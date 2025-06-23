#include "exchange/orderbook.h"
#include "common/utils/logger.h"
#include "nlohmann/json.hpp"

namespace degen_crypto { namespace exchange { namespace order_book {

auto OrderBook::orderbook_message_handler(const std::string& message) -> void {
    try {
        nlohmann::json json_message = nlohmann::json::parse(message);
        LOG_INFO(logger::g_logger, "[{}] Orderbook message: {}", symbol(), json_message.dump());
    } catch (const std::exception& e) {
        LOG_ERROR(logger::g_logger, "Error parsing orderbook message: {}", e.what());
    }
}

} // namespace order_book
} // namespace exchange
} // namespace degen_crypto