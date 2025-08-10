#pragma once
#include <concepts>

namespace degen_crypto {
namespace lib {
namespace exchanges {
template <typename T>
concept ExchangeMarketDataAdapter = requires(T t) {
  { t.on_market_book_update(); }  // snapshot
  { t.on_market_quote_update(); } // tick by tick quote
  { t.on_market_trade_update(); };
};

template <typename T>
concept ExchangeOrderEntryAdapter = requires(T t) {
  { t.get_exchange_name(); }
  { t.send_order(); }
  { t.on_market_new_ack(); }
  { t.on_market_new_reject(); }
  { t.on_market_cancel_ack(); }
  { t.on_market_cancel_reject(); }
  { t.on_market_amend_ack(); }
  { t.on_market_amend_reject(); }
  { t.on_execution();}
};
} // namespace exchanges
} // namespace lib
} // namespace degen_crypto
