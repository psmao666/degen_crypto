#pragma once

#include <string>
#include <nlohmann/json.hpp>
#include <chrono>
#include <iomanip>
#include <sstream>

using namespace nlohmann;

namespace degen_crypto { namespace exchange { namespace binance {

// Response for /api/v3/time
struct ServerTimeResponse {
    int64_t serverTime;
    
    std::chrono::system_clock::time_point get_time_point() const {
        return std::chrono::system_clock::time_point(
            std::chrono::milliseconds(serverTime)
        );
    }
    
    std::string to_string() const {
        auto time = get_time_point();
        auto time_t = std::chrono::system_clock::to_time_t(time);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            time.time_since_epoch()
        ) % 1000;
        
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
        ss << '.' << std::setfill('0') << std::setw(3) << ms.count();
        return ss.str();
    }
    
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(ServerTimeResponse, serverTime)
};

} // namespace binance
} // namespace exchange
} // namespace degen_crypto