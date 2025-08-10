#include <memory>
#include <fmt/core.h>
#include "app_engine.h"

namespace degen_crypto {
namespace lib {
    
template <typename Traits> inline int launch(int argc, char **argv) {
  std::unique_ptr<AppEngine<Traits>> app_engine =
      std::make_unique<AppEngine<Traits>>();
  try {
    if (!app_engine->initialize(argc, argv)) {
      fmt::print("Failed to initialize the application engine.\n");
      return 1;
    }
    if (!app_engine->run()) {
      fmt::print("Failed to run the application engine.\n");
      return 1;
    }
  } catch (const std::exception &e) {
    fmt::print("Exception caught: {}\n", e.what());
    return 1;
  }
  return 0; // Return 0 to indicate success
}
} // namespace lib
} // namespace degen_crypto
