#pragma once

#include "core/Types.h"
#include <cstddef>
#include <cstdint>
#include <vector>

namespace spartak::cluster {

struct DivergenceSignal {
    bool            ok              = false;
    core::OrderSide direction       = core::OrderSide::Buy;  // Buy = бычья, Sell = медвежья
    std::int64_t    ts_pivot1       = 0;     // первый экстремум
    std::int64_t    ts_pivot2       = 0;     // второй экстремум
    double          price_pivot1    = 0.0;
    double          price_pivot2    = 0.0;
    double          rsi_pivot1      = 0.0;
    double          rsi_pivot2      = 0.0;
    double          rsi_current     = 0.0;
    int             bars_between    = 0;
};

struct DivergenceOptions {
    int    rsi_period       = 14;
    int    pivot_window     = 5;    // бар считается экстремумом, если он max/min в окне ±N
    int    search_window    = 50;   // сколько баров назад искать первый экстремум
    double min_price_diff   = 0.0;  // минимальная разница цен между экстремумами
    double min_rsi_diff     = 3.0;  // минимальная разница RSI между экстремумами
};

class DivergenceDetector {
public:
    DivergenceDetector() = default;
    explicit DivergenceDetector(DivergenceOptions opts) : opts_(opts) {}

    void set_options(const DivergenceOptions& opts) { opts_ = opts; }
    const DivergenceOptions& options() const { return opts_; }

    bool validate() const;

    DivergenceSignal find(const std::vector<core::Bar>& bars) const;

private:
    DivergenceOptions opts_{};
};

} // namespace spartak::cluster