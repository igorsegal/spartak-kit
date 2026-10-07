#include "cluster/FalseBreakoutDetector.h"
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <vector>

namespace spartak::cluster {

bool FalseBreakoutDetector::validate() const {
    if (opts_.zone_tolerance_range < 0.001 || opts_.zone_tolerance_range > 0.02) return false;
    if (opts_.min_touches < 2) return false;
    if (opts_.breakout_min_bars < 1) return false;
    if (opts_.return_window < 1) return false;
    if (opts_.window_size < opts_.min_bars_for_tol) return false;
    if (opts_.min_bars_for_tol < 10) return false;
    return true;
}

namespace {

bool touches(const core::Bar& b, double level, double tol) {
    return b.low <= level + tol && b.high >= level - tol;
}

} // namespace

FalseBreakoutSignal
FalseBreakoutDetector::find(const std::vector<core::Bar>& bars) const {
    FalseBreakoutSignal best;

    if (!validate()) return best;

    const std::size_t n = bars.size();
    if (n < static_cast<std::size_t>(opts_.min_bars_for_tol)) return best;

    const std::size_t win = static_cast<std::size_t>(opts_.window_size);
    const std::size_t begin = (n > win) ? (n - win) : 0;
    const std::size_t end = n;

    double min_low = bars[begin].low;
    double max_high = bars[begin].high;
    for (std::size_t i = begin; i < end; ++i) {
        if (bars[i].low  < min_low)  min_low  = bars[i].low;
        if (bars[i].high > max_high) max_high = bars[i].high;
    }
    const double price_range = max_high - min_low;
    if (price_range <= 0.0) return best;

    const double tol = price_range * opts_.zone_tolerance_range;
    if (tol <= 0.0) return best;

    const double ref_price = (max_high + min_low) * 0.5;
    if (ref_price <= 0.0) return best;
    if (tol / ref_price < 1e-6) return best;

    struct LevelCand { double level; };
    std::vector<LevelCand> candidates;

    for (std::size_t i = begin; i < end; ++i) {
        const double values[2] = { bars[i].high, bars[i].low };
        for (int k = 0; k < 2; ++k) {
            const double lv = values[k];
            if (!std::isfinite(lv) || lv <= 0.0) continue;

            bool dup = false;
            for (const auto& c : candidates) {
                if (std::fabs(c.level - lv) <= tol) { dup = true; break; }
            }
            if (dup) continue;

            int cnt = 0;
            for (std::size_t j = begin; j < end; ++j) {
                if (touches(bars[j], lv, tol)) ++cnt;
            }
            if (cnt >= opts_.min_touches) {
                candidates.push_back({ lv });
            }
        }
    }

    for (const auto& cand : candidates) {
        const double level = cand.level;

        // 1. Собираем индексы касаний.
        std::vector<std::size_t> touch_idx;
        for (std::size_t k = begin; k < end; ++k) {
            if (touches(bars[k], level, tol)) touch_idx.push_back(k);
        }
        if (touch_idx.size() < static_cast<std::size_t>(opts_.min_touches)) continue;

        // 2. Момент, когда уровень сформирован (накоплено min_touches касаний).
        const std::size_t level_formed = touch_idx[opts_.min_touches - 1];

        // 3. Ищем пробой ПОСЛЕ формирования уровня. Пробой = переход close через уровень.
        for (std::size_t j = level_formed + 1; j < end; ++j) {
            if (!std::isfinite(bars[j].close)) continue;
            const double prev_close = bars[j - 1].close;
            const double curr_close = bars[j].close;

            const bool break_up   = (prev_close <= level + tol) && (curr_close > level + tol);
            const bool break_down = (prev_close >= level - tol) && (curr_close < level - tol);
            if (!break_up && !break_down) continue;

            const int breakout_dir = break_up ? +1 : -1;
            const std::size_t breakout_idx = j;

            // Касания до пробоя.
            int touches_before = 0;
            std::size_t first_touch = 0;
            bool first_set = false;
            for (std::size_t k = begin; k < breakout_idx; ++k) {
                if (touches(bars[k], level, tol)) {
                    ++touches_before;
                    if (!first_set) { first_touch = k; first_set = true; }
                }
            }
            if (touches_before < opts_.min_touches) continue;

            // Закрепление: breakout_min_bars следующих баров close за уровнем.
            const int need = opts_.breakout_min_bars;
            std::size_t retest_idx = breakout_idx;
            bool confirmed = true;
            for (int k = 1; k <= need; ++k) {
                const std::size_t idx = breakout_idx + static_cast<std::size_t>(k);
                if (idx >= end) { confirmed = false; break; }
                if (!std::isfinite(bars[idx].close)) { confirmed = false; break; }
                if (breakout_dir > 0) {
                    if (bars[idx].close <= level + tol) { confirmed = false; break; }
                } else {
                    if (bars[idx].close >= level - tol) { confirmed = false; break; }
                }
                retest_idx = idx;
            }
            if (!confirmed) continue;

            // Возврат.
            const std::size_t ret_end = std::min(
                retest_idx + static_cast<std::size_t>(opts_.return_window) + 1,
                end);

            bool return_found = false;
            std::size_t return_idx = 0;
            for (std::size_t m = retest_idx + 1; m < ret_end; ++m) {
                if (!std::isfinite(bars[m].close)) continue;
                if (breakout_dir > 0 && bars[m].close < level + tol) {
                    return_found = true;
                    return_idx = m;
                    break;
                }
                if (breakout_dir < 0 && bars[m].close > level - tol) {
                    return_found = true;
                    return_idx = m;
                    break;
                }
            }
            if (!return_found) continue;

            double extreme = (breakout_dir > 0) ? bars[breakout_idx].high : bars[breakout_idx].low;
            for (std::size_t k = breakout_idx + 1; k <= retest_idx; ++k) {
                if (breakout_dir > 0) {
                    if (bars[k].high > extreme) extreme = bars[k].high;
                } else {
                    if (bars[k].low < extreme) extreme = bars[k].low;
                }
            }

            const double move = bars[breakout_idx].close - bars[first_touch].close;
            bool against = false;
            if (std::fabs(move) > tol) {
                if (breakout_dir > 0 && move < 0.0) against = true;
                if (breakout_dir < 0 && move > 0.0) against = true;
            }

            const int bars_out = static_cast<int>(return_idx - retest_idx);

            if (!best.ok || touches_before > best.touches_before) {
                best.ok               = true;
                best.direction        = (breakout_dir > 0) ? core::OrderSide::Sell : core::OrderSide::Buy;
                best.level            = level;
                best.level_ts         = bars[first_touch].timestamp;
                best.breakout_ts      = bars[breakout_idx].timestamp;
                best.retest_ts        = bars[retest_idx].timestamp;
                best.return_ts        = bars[return_idx].timestamp;
                best.breakout_extreme = extreme;
                best.close_at_return  = bars[return_idx].close;
                best.touches_before   = touches_before;
                best.bars_outside     = bars_out;
                best.against_trend    = against;
            }

            break; // Для этого уровня сигнал найден.
        }
    }

    return best;
}

} // namespace spartak::cluster