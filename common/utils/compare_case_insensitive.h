#pragma once

#include <cctype>
#include <string>
#include <string_view>

namespace degen_crypto { namespace common { namespace utils {

static inline bool compare_str_insensitve(const std::string_view strA, const std::string_view strB) {
    if (strA.length() != strB.length()) return false;
    for (int i = 0; i < strA.length(); ++ i) {
        if (std::toupper(strA[i]) != std::toupper(strB[i])) {
            return false;
        }
    }
    return true;
}

} // namespace utils
} // namespace common
} // namespace degen_crypto