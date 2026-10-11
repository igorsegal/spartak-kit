#include "cluster/MirrorLevelDetector.h"
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <vector>

namespace spartak::cluster {

bool MirrorLevelDetector::validate() const {
    if (opts_.zone_tolerance_range <= 0.0 || opts_.zone_tolerance_range >= 1.0) return false;
    if (opts_.min_touches < 2) return false;
    if (opts_.retest_window < 1) return false;
    if (opts_.min_hold_bars < 1) return false;
    if (opts_.pivot_window < 1) return false;
    if (opts_.max_bars_since_retest < 1) return false;
    if (opts_.window_size < opts_.min_touches + opts_.min_hold_bars + opts_.retest_window)
        return false;
    return true;
}

MirrorSignal
MirrorLevelDetector::find(const std::vector<core::Bar>& bars) const {
    return find_impl(bars, 0, bars.size());
}

MirrorSignal
MirrorLevelDetector::find_last(const std::vector<core::Bar>& bars) const {
    if (bars.size() <= static_cast<std::size_t>(opts_.window_size)) {
        return find_impl(bars, 0, bars.size());
    }
    const std::size_t start = bars.size() - static_cast<std::size_t>(opts_.window_size);
    return find_impl(bars, start, bars.size());
}

namespace {

bool touches(const core::Bar& b, double level, double tol) {
    return b.low <= level + tol && b.high >= level - tol;
}

bool is_pivot_high(const std::vector<core::Bar>& bars, std::size_t center, int w) {
    if (center < static_cast<std::size_t>(w)) return false;
    if (center + static_cast<std::size_t>(w) >= bars.size()) return false;
    const double h = bars[center].high;
    for (int k = -w; k <= w; ++k) {
        if (k == 0) continue;
        const std::size_t idx = static_cast<std::size_t>(static_cast<int>(center) + k);
        if (bars[idx].high >= h) return false;
    }
    return true;
}

bool is_pivot_low(const std::vector<core::Bar>& bars, std::size_t center, int w) {
    if (center < static_cast<std::size_t>(w)) return false;
    if (center + static_cast<std::size_t>(w) >= bars.size()) return false;
    const double l = bars[center].low;
    for (int k = -w; k <= w; ++k) {
        if (k == 0) continue;
        const std::size_t idx = static_cast<std::size_t>(static_cast<int>(center) + k);
        if (bars[idx].low <= l) return false;
    }
    return true;
}

} // namespace

MirrorSignal
MirrorLevelDetector::find_impl(const std::vector<core::Bar>& bars,
                               std::size_t search_begin,
                               std::size_t search_end) const {
    MirrorSignal r;

    if (!validate()) return r;

    const std::size_t n = bars.size();
    if (n < static_cast<std::size_t>(opts_.min_touches) + 1) return r;
    if (search_end > n) search_end = n;
    if (search_begin >= search_end) return r;

    double min_low  = bars[search_begin].low;
    double max_high = bars[search_begin].high;
    for (std::size_t i = search_begin; i < search_end; ++i) {
        if (bars[i].low  < min_low)  min_low  = bars[i].low;
        if (bars[i].high > max_high) max_high = bars[i].high;
    }
    const double price_range = max_high - min_low;
    if (price_range <= 0.0) return r;
    const double tol = price_range * opts_.zone_tolerance_range;

    MirrorSignal best;
    int best_score = 0;

    struct LevelCand { double level; };
    std::vector<LevelCand> candidates;

    for (std::size_t i = search_begin; i < search_end; ++i) {
        const bool ph = is_pivot_high(bars, i, opts_.pivot_window);
        const bool pl = is_pivot_low(bars, i, opts_.pivot_window);
        if (!ph && !pl) continue;

        const double lv = ph ? bars[i].high : bars[i].low;
        if (lv <= 0.0) continue;

        bool dup = false;
        for (const auto& c : candidates) {
            if (std::fabs(c.level - lv) <= tol) { dup = true; break; }
        }
        if (dup) continue;

        int cnt = 0;
        for (std::size_t j = search_begin; j < search_end; ++j) {
            if (touches(bars[j], lv, tol)) ++cnt;
        }
        if (cnt >= opts_.min_touches) {
            candidates.push_back({ lv });
        }
    }

    for (const auto& cand : candidates) {
        const double level = cand.level;

        for (std::size_t j = search_begin + 1; j < search_end; ++j) {
            const auto& bj = bars[j];

            const bool break_up   = bj.close > level + tol;
            const bool break_down = bj.close < level - tol;
            if (!break_up && !break_down) continue;

            int touches_before = 0;
            std::size_t first_touch = 0;
            std::size_t last_touch  = 0;
            bool first_set = false;
            bool broken_inside = false;

            for (std::size_t k = search_begin; k < j; ++k) {
                const auto& bk = bars[k];

                if (first_set) {
                    if (break_up && bk.close > level + tol) { broken_inside = true; break; }
                    if (break_down && bk.close < level - tol) { broken_inside = true; break; }
                }

                if (touches(bk, level, tol)) {
                    ++touches_before;
                    last_touch = k;
                    if (!first_set) { first_touch = k; first_set = true; }
                }
            }

            if (broken_inside) continue;
            if (touches_before < opts_.min_touches) continue;

            const std::size_t hold_bars = j - first_touch;
            if (hold_bars < static_cast<std::size_t>(opts_.min_hold_bars)) continue;

            const std::size_t retest_end = std::min(
                j + static_cast<std::size_t>(opts_.retest_window) + 1,
                search_end);

            bool retest_found = false;
            std::size_t retest_idx = 0;
            bool retest_ok = false;

            for (std::size_t m = j + 1; m < retest_end; ++m) {
                const auto& bm = bars[m];
                if (touches(bm, level, tol)) {
                    retest_found = true;
                    retest_idx = m;
                    if (break_up   && bm.close > level) retest_ok = true;
                    if (break_down && bm.close < level) retest_ok = true;
                    break;
                }
            }

            if (!retest_found) continue;

            const std::size_t bars_after_retest = (search_end - 1) - retest_idx;
            if (bars_after_retest > static_cast<std::size_t>(opts_.max_bars_since_retest)) {
                continue;
            }

            core::OrderSide dir = break_up ? core::OrderSide::Buy : core::OrderSide::Sell;

            int score = touches_before;
            if (retest_ok) score += 5;

            if (score > best_score) {
                best_score = score;
                best.ok                   = true;
                best.direction            = dir;
                best.level                = level;
                best.hold_start_ts        = bars[first_touch].timestamp;
                best.hold_end_ts          = bars[last_touch].timestamp;
                best.break_ts             = bars[j].timestamp;
                best.retest_ts            = bars[retest_idx].timestamp;
                best.retest_ok            = retest_ok;
                best.touches_before_break = touches_before;
                best.bars_since_break     = static_cast<int>(retest_idx - j);
            }

            break;
        }
    }

    return best;
}

} // namespace spartak::cluster