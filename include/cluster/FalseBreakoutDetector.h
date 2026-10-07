#pragma once

#include "core/Types.h"
#include <cstddef>
#include <cstdint>
#include <vector>

namespace spartak::cluster {

struct FalseBreakoutSignal {
    bool            ok               = false;
    core::OrderSide direction        = core::OrderSide::Buy;
    double          level            = 0.0;
    std::int64_t    level_ts         = 0;
    std::int64_t    breakout_ts      = 0;
    std::int64_t    retest_ts        = 0;
    std::int64_t    return_ts        = 0;
    double          breakout_extreme = 0.0;
    double          close_at_return  = 0.0;
    int             touches_before   = 0;
    int             bars_outside     = 0;
    bool            against_trend    = false;
};

struct FalseBreakoutOptions {
    double zone_tolerance_range = 0.01;
    int    min_touches          = 3;
    int    breakout_min_bars    = 2;
    int    return_window        = 5;
    int    window_size          = 200;
    int    min_bars_for_tol     = 40;
};

class FalseBreakoutDetector {
public:
    FalseBreakoutDetector() = default;
    explicit FalseBreakoutDetector(FalseBreakoutOptions opts) : opts_(opts) {}

    void set_options(const FalseBreakoutOptions& opts) { opts_ = opts; }
    const FalseBreakoutOptions& options() const { return opts_; }

    bool validate() const;

    FalseBreakoutSignal find(const std::vector<core::Bar>& bars) const;

private:
    FalseBreakoutOptions opts_{};
};

} // namespace spartak::cluster