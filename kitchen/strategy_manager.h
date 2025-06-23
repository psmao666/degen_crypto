#pragma once

#include <string>
#include <vector>
#include <memory>
#include <algorithm>
#include <nlohmann/json_fwd.hpp>
#include "common/utils/logger.h"

namespace degen_crypto { namespace kitchen {

class StrategyManager;

/**
 * @brief An abstract interface for all strategies.
 * This allows the StrategyManager to hold different concrete strategies
 * in a single polymorphic collection.
 */
class IStrategy {
public:
    virtual ~IStrategy() = default;
    virtual bool on_start() = 0;
    virtual bool on_shutdown() = 0;
    virtual bool load_params(const nlohmann::json& params) = 0;
    virtual const std::string& strategy_name() const = 0;
    virtual void run() = 0;
    void set_manager(StrategyManager* manager) { manager_ = manager; }

protected:
    StrategyManager* manager_ = nullptr;
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

    // Override the virtual functions from IStrategy and delegate to Derived.
    bool on_start() override { return derived().on_start(); }
    bool on_shutdown() override { return derived().on_shutdown(); }
    bool load_params(const nlohmann::json& params) override { return derived().load_params(params); }
    const std::string& strategy_name() const override { return derived().strategy_name(); }
    void run() override { derived().run(); }

protected:
    auto derived() -> Derived& { return static_cast<Derived&>(*this); }
    auto derived() const -> const Derived& { return static_cast<const Derived&>(*this); }
};

/**
 * @brief Manages the lifecycle of all active trading strategies.
 */
class StrategyManager {
public:
    StrategyManager() = default;
    ~StrategyManager() {
        // Ensure all strategies are shutdown cleanly on destruction.
        for (auto& strategy : strategies_) {
            strategy->on_shutdown();
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
            LOG_WARN(logger::g_logger, "Strategy '{}' is already hooked.", name);
            return false;
        }

        new_strategy->set_manager(this);

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