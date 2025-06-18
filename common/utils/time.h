#pragma once

#include <chrono>

namespace degen_crypto { namespace common { namespace utils {

inline std::uint64_t get_current_ms_epoch() {
    return static_cast<std::uint64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count());
}

} // namespace utils
} // namespace common
} // namespace degen_crypto