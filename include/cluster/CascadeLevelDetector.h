#pragma once

#include "core/Types.h"
#include <cstddef>
#include <cstdint>
#include <vector>

namespace spartak::cluster {

struct CascadeSignal {
    bool            ok            = false;
    core::OrderSide direction     = core::OrderSide::Buy;
    double          level_a       = 0.0;
    double          level_b       = 0.0;
    std::int64_t    level_a_ts    = 0;
    std::int64_t    level_b_ts    = 0;
    std::int64_t    rebound_a_ts  = 0;
    std::int64_t    rebound_b_ts  = 0;
    int             touches_a     = 0;
    int             touches_b     = 0;
    int             bars_between  = 0;
};

struct CascadeOptions {
    double zone_tolerance_range = 0.01;
    int    min_bars_for_cascade = 20;
    int    min_touches_per_level = 1;
    int    max_gap_bars         = 50;
    int    max_rebound_bars     = 10;
    double min_rebound_pct      = 0.5;
    int    window_size          = 200;
};

class CascadeLevelDetector {
public:
    CascadeLevelDetector() = default;
    explicit CascadeLevelDetector(CascadeOptions opts) : opts_(opts) {}

    void set_options(const CascadeOptions& opts) { opts_ = opts; }
    const CascadeOptions& options() const { return opts_; }

    bool validate() const;

    CascadeSignal find(const std::vector<core::Bar>& bars) const;

private:
    CascadeOptions opts_{};
};

} // namespace spartak::cluster