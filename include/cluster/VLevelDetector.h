#pragma once

#include "core/Types.h"
#include <cstddef>
#include <cstdint>
#include <vector>

namespace spartak::cluster {

struct VLevelSignal {
    bool              ok              = false;
    core::OrderSide   direction       = core::OrderSide::Buy;
    std::int64_t      start_ts        = 0;
    std::int64_t      end_ts          = 0;
    double            start_price     = 0.0;
    double            end_price       = 0.0;
    double            level_0         = 0.0;
    double            level_50        = 0.0;
    double            level_100       = 0.0;
    int               bars_in_impulse = 0;
    double            impulse_pct     = 0.0;
    bool              is_broken       = false;
    bool              in_zone_50      = false;
    bool              in_zone_100     = false;
};

struct VLevelOptions {
    int    min_impulse_bars = 2;
    int    max_impulse_bars = 9;
    double one_way_ratio    = 0.7;
    double min_strength     = 0.003;
    double zone_tolerance   = 0.05;
    int    window_size      = 100;
};

class VLevelDetector {
public:
    VLevelDetector() = default;
    explicit VLevelDetector(VLevelOptions opts) : opts_(opts) {}

    void set_options(const VLevelOptions& opts) { opts_ = opts; }
    const VLevelOptions& options() const { return opts_; }

    VLevelSignal find(const std::vector<core::Bar>& bars) const;
    VLevelSignal find_last(const std::vector<core::Bar>& bars) const;

private:
    VLevelSignal find_impl(const std::vector<core::Bar>& bars,
                           std::size_t search_begin,
                           std::size_t search_end) const;

    VLevelOptions opts_{};
};

} // namespace spartak::cluster