#pragma once

#include "core/Types.h"
#include <cstddef>
#include <cstdint>
#include <vector>

namespace spartak::cluster {

struct VolumeProfileSignal {
    bool            ok              = false;
    core::OrderSide direction       = core::OrderSide::Buy;
    double          poc             = 0.0;
    double          value_area_high = 0.0;
    double          value_area_low  = 0.0;
    double          ask_total       = 0.0;
    double          bid_total       = 0.0;
    double          imbalance       = 0.0;
    int             bins_used       = 0;
    double          total_volume    = 0.0;
};

struct VolumeProfileOptions {
    double value_area_pct       = 0.70;
    int    min_bars_for_profile = 20;
    double neutral_threshold    = 0.05;
};

class VolumeProfileFilter {
public:
    VolumeProfileFilter() = default;
    explicit VolumeProfileFilter(VolumeProfileOptions opts) : opts_(opts) {}

    void set_options(const VolumeProfileOptions& opts) { opts_ = opts; }
    const VolumeProfileOptions& options() const { return opts_; }

    bool validate() const;

    VolumeProfileSignal find(const std::vector<core::Bar>& bars) const;

private:
    VolumeProfileOptions opts_{};
};

} // namespace spartak::cluster