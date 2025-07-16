#pragma once

#include "nlohmann/json.hpp"

namespace degen_crypto { namespace kitchen {

struct Buy_Level {
    double price;
    double buy_capital_ratio;

    void from_json(const nlohmann::json& j) {
        price = j.at("price").get<double>();
        buy_capital_ratio = j.at("buy_capital_ratio").get<double>();
    }
};

struct Sell_Level {
    double price;
    double sell_capital_ratio;

    void from_json(const nlohmann::json& j) {
        price = j.at("price").get<double>();
        sell_capital_ratio = j.at("sell_capital_ratio").get<double>();
    }
};

struct USDCUSDT_Grid_Config {
    std::vector<Buy_Level> buy_levels;
    std::vector<Sell_Level> sell_levels;

    bool load_params(const nlohmann::json& params) {
        if (params.contains("buy_levels")) {
            for (const auto& level : params["buy_levels"]) {
                buy_levels.emplace_back();
                buy_levels.back().from_json(level);
            }
        } else {
            return false;
        }
        if (params.contains("sell_levels")) {
            for (const auto& level : params["sell_levels"]) {
                sell_levels.emplace_back();
                sell_levels.back().from_json(level);
            }
        } else {
            return false;
        }
        return true;
    }
};
} // namespace kitchen
} // namespace degen_crypto