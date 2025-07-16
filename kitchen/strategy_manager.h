#pragma once

#include <string>
#include <vector>
#include <memory>
#include <algorithm>
#include <nlohmann/json_fwd.hpp>
#include "common/utils/logger.h"
#include "exchange/orderbook.h"

namespace degen_crypto { namespace kitchen {

class StrategyManager;

/**
 * @brief An abstract interface for all strategies.
 * This allows the StrategyManager to hold different concrete strategies
 * in a single polymorphic collection.
 */
class IStrategy {
public:
    typedef exchange::order_book::OrderBook order_book_t;
public:
    virtual ~IStrategy() = default;
    virtual bool on_start() = 0;
    virtual bool on_shutdown() = 0;
    virtual bool load_params(const nlohmann::json& params) = 0;
    virtual const char* strategy_name() const = 0;
    virtual void run() = 0;
    virtual void on_orderbook_update_callback(const std::string& exchange_name, const std::string& symbol, const order_book_t& orderbook) = 0;

};

/**
 * @brief The CRTP base class for a concrete strategy.
 * Implements the IStrategy interface and forwards calls to the Derived type.
 */
template <typename Derived>
class Strategy : public IStrategy {
public:
    Strategy() = default;
    ~Strategy() override = default;

    bool on_start() override { return derived().on_start(); }
    bool on_shutdown() override { return derived().on_shutdown(); }
    bool load_params(const nlohmann::json& params) override { return derived().load_params(params); }
    const char* strategy_name() const override { return derived().strategy_name(); }
    void run() override { derived().run(); }
    void on_orderbook_update_callback(const std::string& exchange_name, const std::string& symbol, const order_book_t& orderbook) override {
        derived().on_orderbook_update_callback(exchange_name, symbol, orderbook);
    }

protected:
    auto derived() -> Derived& { return static_cast<Derived&>(*this); }
    auto derived() const -> const Derived& { return static_cast<const Derived&>(*this); }
};

/**
 * @brief Manages the lifecycle of all active trading strategies.
 */
class StrategyManager {
public:
    typedef exchange::order_book::OrderBook order_book_t;
public:
    StrategyManager();
    ~StrategyManager() {
        // Ensure all strategies are shutdown cleanly on destruction.
        for (auto& strategy : strategies_) {
            strategy->on_shutdown();
        }
    }

    /**
     * @brief Notify all strategies about an orderbook update
     */
    void notify_orderbook_update(const std::string& exchange_name, const std::string& symbol, const order_book_t& orderbook) {
        for (auto& strategy : strategies_) {
            try {
                strategy->on_orderbook_update_callback(exchange_name, symbol, orderbook);
            } catch (const std::exception& e) {
                LOG_ERROR(logger::g_logger, "Strategy '{}' failed in orderbook callback: {}", 
                         strategy->strategy_name(), e.what());
            }
        }
    }

    /**
     * @brief Hooks and starts a new strategy.
     * @tparam StrategyType The concrete strategy class to create.
     * @param params JSON object with parameters for the strategy.
     * @return True if the strategy was hooked successfully, false otherwise.
     */
    template <typename StrategyType>
    bool hook(const nlohmann::json& params) {
        auto new_strategy = std::make_unique<StrategyType>();
        const auto& name = new_strategy->strategy_name();
        
        if (std::any_of(strategies_.begin(), strategies_.end(), 
            [&name](const auto& s) { return s->strategy_name() == name; })) {
            LOG_WARNING(logger::g_logger, "Strategy '{}' is already hooked.", name);
            return false;
        }

        if (!new_strategy->load_params(params)) {
            LOG_ERROR(logger::g_logger, "Failed to load params for strategy '{}'", name);
            return false;
        }

        if (!new_strategy->on_start()) {
            LOG_ERROR(logger::g_logger, "Failed to start strategy '{}'", name);
            return false;
        }

        strategies_.push_back(std::move(new_strategy));
        LOG_INFO(logger::g_logger, "Strategy '{}' hooked successfully.", name);
        return true;
    }

    /**
     * @brief Shuts down and unhooks a strategy by its name.
     * @param strategy_name The name of the strategy to remove.
     * @return True if the strategy was found and unhooked, false otherwise.
     */
    bool unhook(const std::string& strategy_name) {
        auto it = std::find_if(strategies_.begin(), strategies_.end(), 
                             [&strategy_name](const auto& s) { return s->strategy_name() == strategy_name; });

        if (it == strategies_.end()) {
            LOG_WARNING(logger::g_logger, "Strategy '{}' not found for unhooking.", strategy_name);
            return false;
        }

        (*it)->on_shutdown();
        strategies_.erase(it);
        LOG_INFO(logger::g_logger, "Strategy '{}' unhooked successfully.", strategy_name);
        return true;
    }

private:
    std::vector<std::unique_ptr<IStrategy>> strategies_;
};

} // namespace kitchen
} // namespace degen_crypto