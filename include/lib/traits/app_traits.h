#pragma once
#include <ratio>

namespace degen_crypto {
namespace lib {
namespace traits {
struct AppTraits {
    // fixme: we should use decimal (fixed point) instead of floating point
    using Price = double;
    using Quantity = double;
};
} // namespace traits
} // namespace lib
} // namespace degen_crypto
