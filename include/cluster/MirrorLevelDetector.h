#pragma once

#include "core/Types.h"
#include <cstddef>
#include <cstdint>
#include <vector>

namespace spartak::cluster {

struct MirrorSignal {
    bool            ok                   = false;
    core::OrderSide direction            = core::OrderSide::Buy;
    double          level                = 0.0;
    std::int64_t    hold_start_ts        = 0;
    std::int64_t    hold_end_ts          = 0;
    std::int64_t    break_ts             = 0;
    std::int64_t    retest_ts            = 0;
    bool            retest_ok            = false;
    int             touches_before_break = 0;
    int             bars_since_break     = 0;
};

struct MirrorOptions {
    double zone_tolerance_range = 0.01;
    int    min_touches          = 3;
    int    retest_window        = 20;
    int    min_hold_bars        = 5;
    int    window_size          = 200;
};

class MirrorLevelDetector {
public:
    MirrorLevelDetector() = default;
    explicit MirrorLevelDetector(MirrorOptions opts) : opts_(opts) {}

    void set_options(const MirrorOptions& opts) { opts_ = opts; }
    const MirrorOptions& options() const { return opts_; }

    bool validate() const;

    MirrorSignal find(const std::vector<core::Bar>& bars) const;
    MirrorSignal find_last(const std::vector<core::Bar>& bars) const;

private:
    MirrorSignal find_impl(const std::vector<core::Bar>& bars,
                           std::size_t search_begin,
                           std::size_t search_end) const;

    MirrorOptions opts_{};
};

} // namespace spartak::cluster