#pragma once

#include "core/Types.h"
#include <cstddef>
#include <cstdint>
#include <vector>

namespace spartak::cluster {

struct StopHuntSignal {
    bool            ok             = false;
    core::OrderSide direction      = core::OrderSide::Buy;
    double          level          = 0.0;
    std::int64_t    level_ts       = 0;
    std::int64_t    touch_ts       = 0;
    std::int64_t    pierce_ts      = 0;
    std::int64_t    return_ts      = 0;
    double          pierce_extreme = 0.0;
    double          close_at_return = 0.0;
    int             touches_before = 0;
    int             bars_outside   = 0;
};

struct StopHuntOptions {
    double zone_tolerance_range = 0.01;
    int    min_bars_for_hunt    = 20;
    int    min_touches          = 2;
    int    max_pierce_bars      = 3;
    int    window_size          = 200;
};

class StopHuntDetector {
public:
    StopHuntDetector() = default;
    explicit StopHuntDetector(StopHuntOptions opts) : opts_(opts) {}

    void set_options(const StopHuntOptions& opts) { opts_ = opts; }
    const StopHuntOptions& options() const { return opts_; }

    bool validate() const;

    StopHuntSignal find(const std::vector<core::Bar>& bars) const;

private:
    StopHuntOptions opts_{};
};

} // namespace spartak::cluster