#pragma once

#include "core/Types.h"
#include <cstddef>
#include <vector>

namespace spartak::cluster {

struct CompositeOptions {
    int min_weight   = 5;
    int min_factors  = 2;
    int min_lead     = 2;
    int window_size  = 500;
};

struct CompositeResult {
    bool            ok                = false;
    core::OrderSide direction         = core::OrderSide::Buy;
    int             total_weight      = 0;
    int             opposite_weight   = 0;
    int             factors_count     = 0;
    bool            range_fired       = false;
    bool            vlevel_fired      = false;
    bool            mirror_fired      = false;
    bool            divergence_fired  = false;
    bool            false_breakout_fired = false;
    bool            cascade_fired     = false;
    bool            stop_hunt_fired   = false;
};

class CompositeSignal {
public:
    CompositeSignal() = default;
    explicit CompositeSignal(CompositeOptions opts) : opts_(opts) {}

    void set_options(const CompositeOptions& opts) { opts_ = opts; }
    const CompositeOptions& options() const { return opts_; }

    bool validate() const;

    CompositeResult find(const std::vector<core::Bar>& bars) const;

private:
    CompositeOptions opts_{};
};

} // namespace spartak::cluster