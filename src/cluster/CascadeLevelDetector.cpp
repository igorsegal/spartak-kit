#include "cluster/CascadeLevelDetector.h"
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <vector>

namespace spartak::cluster {

bool CascadeLevelDetector::validate() const {
    if (opts_.zone_tolerance_range <= 0.0 || opts_.zone_tolerance_range >= 1.0) return false;
    if (opts_.min_bars_for_cascade < 10) return false;
    if (opts_.min_touches_per_level < 1) return false;
    if (opts_.max_gap_bars < 1) return false;
    if (opts_.max_rebound_bars < 1) return false;
    if (opts_.min_rebound_pct <= 0.0 || opts_.min_rebound_pct >= 1.0) return false;
    if (opts_.window_size < opts_.min_bars_for_cascade) return false;
    if (opts_.window_size < 1) return false;
    if (opts_.window_size > 100000) return false;
    return true;
}

namespace {

bool is_finite_bar(const core::Bar& b) {
    if (!std::isfinite(b.open)) return false;
    if (!std::isfinite(b.high)) return false;
    if (!std::isfinite(b.low)) return false;
    if (!std::isfinite(b.close)) return false;
    if (!std::isfinite(b.volume)) return false;
    if (!std::isfinite(b.ask_volume)) return false;
    if (!std::isfinite(b.bid_volume)) return false;
    return true;
}

bool is_valid_bar(const core::Bar& b) {
    if (!is_finite_bar(b)) return false;
    if (b.high < b.low) return false;
    if (b.ask_volume < 0.0) return false;
    if (b.bid_volume < 0.0) return false;
    if (b.volume < 0.0) return false;
    if (b.volume > 0.0 && (b.ask_volume + b.bid_volume) == 0.0) return false;
    return true;
}

bool touches(const core::Bar& b, double level, double tol) {
    return b.low <= level + tol && b.high >= level - tol;
}

struct LevelInfo {
    double level;
    std::vector<std::size_t> touches;
    std::size_t last_touch_idx;
    std::size_t rebound_idx;
    bool valid;
};

bool find_rebound(const std::vector<core::Bar>& bars,
                  double level,
                  std::size_t last_touch,
                  int max_rebound_bars,
                  double min_rebound_dist,
                  std::size_t& out_idx)
{
    const std::size_t n = bars.size();
    const std::size_t end = std::min(
        last_touch + 1 + static_cast<std::size_t>(max_rebound_bars),
        n);

    for (std::size_t i = last_touch + 1; i < end; ++i) {
        const double c = bars[i].close;
        const double c_touch = bars[last_touch].close;
        if (c_touch < level) {
            if (level - c >= min_rebound_dist) { out_idx = i; return true; }
        } else {
            if (c - level >= min_rebound_dist) { out_idx = i; return true; }
        }
    }
    return false;
}

} // namespace

CascadeSignal
CascadeLevelDetector::find(const std::vector<core::Bar>& bars) const {
    CascadeSignal r;

    if (!validate()) return r;

    std::vector<core::Bar> valid;
    valid.reserve(bars.size());
    for (const auto& b : bars) {
        if (is_valid_bar(b)) valid.push_back(b);
    }

    const std::size_t n_valid = valid.size();
    if (n_valid < static_cast<std::size_t>(opts_.min_bars_for_cascade)) return r;

    if (n_valid > static_cast<std::size_t>(opts_.window_size)) {
        valid.erase(valid.begin(), valid.end() - opts_.window_size);
    }

    const std::size_t n = valid.size();

    double min_low = valid[0].low;
    double max_high = valid[0].high;
    for (const auto& b : valid) {
        if (b.low < min_low) min_low = b.low;
        if (b.high > max_high) max_high = b.high;
    }
    const double range = max_high - min_low;
    if (range <= 0.0) return r;

    const double tol = range * opts_.zone_tolerance_range;
    if (tol <= 0.0 || tol > range) return r;

    std::vector<double> cands;
    for (const auto& b : valid) {
        if (b.high == b.low) continue;
        cands.push_back(b.high);
        cands.push_back(b.low);
    }

    std::vector<double> levels;
    for (double lv : cands) {
        if (lv <= 0.0) continue;
        bool dup = false;
        for (double existing : levels) {
            if (std::fabs(existing - lv) <= tol) { dup = true; break; }
        }
        if (!dup) levels.push_back(lv);
    }

    std::vector<LevelInfo> infos;
    infos.reserve(levels.size());

    const double min_rebound_dist = opts_.min_rebound_pct * range;

    for (double lv : levels) {
        LevelInfo info;
        info.level = lv;
        info.valid = false;
        info.last_touch_idx = 0;
        info.rebound_idx = 0;

        for (std::size_t i = 0; i < n; ++i) {
            if (touches(valid[i], lv, tol)) info.touches.push_back(i);
        }

        if (static_cast<int>(info.touches.size()) < opts_.min_touches_per_level) {
            continue;
        }

        info.last_touch_idx = info.touches.back();

        std::size_t reb_idx = 0;
        if (!find_rebound(valid, lv, info.last_touch_idx,
                          opts_.max_rebound_bars, min_rebound_dist, reb_idx)) {
            continue;
        }
        info.rebound_idx = reb_idx;
        info.valid = true;
        infos.push_back(info);
    }

    if (infos.size() < 2) return r;

    const double last_close = valid.back().close;

    struct Pair {
        std::size_t ia;
        std::size_t ib;
        int score;
        std::size_t idx_a;
        std::size_t idx_b;
        bool found;
    };
    Pair best{0, 0, -1, 0, 0, false};

    for (std::size_t i = 0; i < infos.size(); ++i) {
        for (std::size_t j = i + 1; j < infos.size(); ++j) {
            const LevelInfo& li = infos[i];
            const LevelInfo& lj = infos[j];

            const bool side_i = li.level > last_close;
            const bool side_j = lj.level > last_close;
            if (side_i == side_j) continue;

            const std::size_t ra = li.rebound_idx;
            const std::size_t rb = lj.rebound_idx;
            const std::size_t gap = (ra > rb) ? (ra - rb) : (rb - ra);
            if (gap > static_cast<std::size_t>(opts_.max_gap_bars)) continue;

            const int score = static_cast<int>(li.touches.size() + lj.touches.size());

            std::size_t ia = i;
            std::size_t ib = j;
            if (infos[ia].level < infos[ib].level) std::swap(ia, ib);

            if (score > best.score ||
                (score == best.score && (!best.found || ia < best.idx_a ||
                 (ia == best.idx_a && ib < best.idx_b)))) {
                best.ia = ia;
                best.ib = ib;
                best.score = score;
                best.idx_a = ia;
                best.idx_b = ib;
                best.found = true;
            }
        }
    }

    if (!best.found) return r;

    const LevelInfo& a = infos[best.ia];
    const LevelInfo& b = infos[best.ib];

    r.ok           = true;
    r.direction    = core::OrderSide::Sell;
    r.level_a      = a.level;
    r.level_b      = b.level;
    r.level_a_ts   = valid[a.touches.front()].timestamp;
    r.level_b_ts   = valid[b.touches.front()].timestamp;
    r.rebound_a_ts = valid[a.rebound_idx].timestamp;
    r.rebound_b_ts = valid[b.rebound_idx].timestamp;
    r.touches_a    = static_cast<int>(a.touches.size());
    r.touches_b    = static_cast<int>(b.touches.size());
    r.bars_between = static_cast<int>(a.rebound_idx > b.rebound_idx
                                      ? a.rebound_idx - b.rebound_idx
                                      : b.rebound_idx - a.rebound_idx);

    return r;
}

} // namespace spartak::cluster