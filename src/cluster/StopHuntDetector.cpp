#include "cluster/StopHuntDetector.h"
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <vector>

namespace spartak::cluster {

bool StopHuntDetector::validate() const {
    if (opts_.zone_tolerance_range <= 0.0 || opts_.zone_tolerance_range >= 1.0) return false;
    if (opts_.min_bars_for_hunt < 10) return false;
    if (opts_.min_touches < 1) return false;
    if (opts_.max_pierce_bars < 1 || opts_.max_pierce_bars > 10) return false;
    if (opts_.window_size < opts_.min_bars_for_hunt) return false;
    if (opts_.window_size < 1) return false;
    if (opts_.window_size > 20000) return false;
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

} // namespace

StopHuntSignal
StopHuntDetector::find(const std::vector<core::Bar>& bars) const {
    StopHuntSignal r;

    if (!validate()) return r;

    std::vector<core::Bar> valid;
    valid.reserve(bars.size());
    for (const auto& b : bars) {
        if (is_valid_bar(b)) valid.push_back(b);
    }

    if (valid.size() < static_cast<std::size_t>(opts_.min_bars_for_hunt)) return r;

    if (valid.size() > static_cast<std::size_t>(opts_.window_size)) {
        valid.erase(valid.begin(), valid.end() - opts_.window_size);
    }

    const std::size_t n = valid.size();
    if (n < static_cast<std::size_t>(opts_.min_bars_for_hunt)) return r;

    double min_low = valid[0].low;
    double max_high = valid[0].high;
    for (const auto& b : valid) {
        if (b.low < min_low) min_low = b.low;
        if (b.high > max_high) max_high = b.high;
    }
    const double range = max_high - min_low;
    if (range <= 0.0) return r;

    const double tol = range * opts_.zone_tolerance_range;
    if (tol <= 0.0 || tol >= range) return r;

    std::vector<double> cands;
    for (const auto& b : valid) {
        if (b.high == b.low) continue;
        cands.push_back(b.high);
        cands.push_back(b.low);
    }

    std::vector<double> levels;
    for (double lv : cands) {
        if (!std::isfinite(lv) || lv <= 0.0) continue;
        bool dup = false;
        for (double existing : levels) {
            if (std::fabs(existing - lv) <= tol) { dup = true; break; }
        }
        if (!dup) levels.push_back(lv);
    }

    struct Hunt {
        double level;
        std::size_t pierce_idx;
        int pierce_dir;
        std::size_t return_idx;
        std::size_t first_touch_idx;
        std::size_t last_touch_idx;
        int touches_before;
    };

    std::vector<Hunt> found;

    for (double lv : levels) {
        // Инкрементальный счётчик касаний. Один проход по барам.
        int touches_so_far = 0;
        std::size_t first_touch = 0;
        std::size_t last_touch = 0;
        bool any_touch = false;

        for (std::size_t j = 1; j < n; ++j) {
            // Обновляем счётчик касаний для бара j-1.
            if (touches(valid[j - 1], lv, tol)) {
                ++touches_so_far;
                if (!any_touch) { first_touch = j - 1; any_touch = true; }
                last_touch = j - 1;
            }

            // Проверяем пробой на баре j.
            const double prev_close = valid[j - 1].close;
            const double curr_close = valid[j].close;

            int pierce_dir = 0;
            if (curr_close > lv + tol && prev_close <= lv + tol) pierce_dir = +1;
            else if (curr_close < lv - tol && prev_close >= lv - tol) pierce_dir = -1;
            if (pierce_dir == 0) continue;

            if (touches_so_far < opts_.min_touches) continue;

            // Возврат.
            const std::size_t return_end = std::min(
                j + 1 + static_cast<std::size_t>(opts_.max_pierce_bars),
                n);

            bool returned = false;
            std::size_t return_idx = 0;
            for (std::size_t m = j + 1; m < return_end; ++m) {
                const double c = valid[m].close;
                if (pierce_dir > 0 && c < lv + tol) { returned = true; return_idx = m; break; }
                if (pierce_dir < 0 && c > lv - tol) { returned = true; return_idx = m; break; }
            }
            if (!returned) continue;

            Hunt h;
            h.level = lv;
            h.pierce_idx = j;
            h.pierce_dir = pierce_dir;
            h.return_idx = return_idx;
            h.first_touch_idx = first_touch;
            h.last_touch_idx = last_touch;
            h.touches_before = touches_so_far;
            found.push_back(h);
            break;  // Первый подходящий пробой для уровня.
        }
    }

    if (found.empty()) return r;

    std::size_t best = 0;
    for (std::size_t i = 1; i < found.size(); ++i) {
        if (found[i].touches_before > found[best].touches_before) best = i;
    }

    const Hunt& b = found[best];

    double extreme = (b.pierce_dir > 0) ? valid[b.pierce_idx].high : valid[b.pierce_idx].low;
    for (std::size_t k = b.last_touch_idx; k <= b.pierce_idx; ++k) {
        if (b.pierce_dir > 0) {
            if (valid[k].high > extreme) extreme = valid[k].high;
        } else {
            if (valid[k].low < extreme) extreme = valid[k].low;
        }
    }

    r.ok              = true;
    r.direction       = (b.pierce_dir > 0) ? core::OrderSide::Sell : core::OrderSide::Buy;
    r.level           = b.level;
    r.level_ts        = valid[b.first_touch_idx].timestamp;
    r.touch_ts        = valid[b.last_touch_idx].timestamp;
    r.pierce_ts       = valid[b.pierce_idx].timestamp;
    r.return_ts       = valid[b.return_idx].timestamp;
    r.pierce_extreme  = extreme;
    r.close_at_return = valid[b.return_idx].close;
    r.touches_before  = b.touches_before;
    r.bars_outside    = static_cast<int>(b.return_idx - b.pierce_idx);

    return r;
}

} // namespace spartak::cluster