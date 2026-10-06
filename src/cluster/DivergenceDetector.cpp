#include "cluster/DivergenceDetector.h"
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <vector>

namespace spartak::cluster {

bool DivergenceDetector::validate() const {
    if (opts_.rsi_period < 2) return false;
    if (opts_.pivot_window < 1) return false;
    if (opts_.search_window < opts_.pivot_window * 2 + 1) return false;
    if (opts_.min_price_diff < 0.0) return false;
    if (opts_.min_rsi_diff < 0.0) return false;
    return true;
}

namespace {

std::vector<double> compute_rsi(const std::vector<core::Bar>& bars, int period) {
    std::vector<double> rsi(bars.size(), 0.0);
    if (bars.size() < static_cast<std::size_t>(period) + 1) return rsi;

    double gain_sum = 0.0;
    double loss_sum = 0.0;
    for (int i = 1; i <= period; ++i) {
        const double diff = bars[i].close - bars[i - 1].close;
        if (diff > 0.0) gain_sum += diff;
        else loss_sum += -diff;
    }
    double avg_gain = gain_sum / period;
    double avg_loss = loss_sum / period;

    auto rsi_from = [](double ag, double al) -> double {
        if (al == 0.0) return 100.0;
        const double rs = ag / al;
        return 100.0 - 100.0 / (1.0 + rs);
    };

    rsi[period] = rsi_from(avg_gain, avg_loss);

    for (std::size_t i = static_cast<std::size_t>(period) + 1; i < bars.size(); ++i) {
        const double diff = bars[i].close - bars[i - 1].close;
        const double gain = (diff > 0.0) ? diff : 0.0;
        const double loss = (diff < 0.0) ? -diff : 0.0;
        avg_gain = (avg_gain * (period - 1) + gain) / period;
        avg_loss = (avg_loss * (period - 1) + loss) / period;
        rsi[i] = rsi_from(avg_gain, avg_loss);
    }

    return rsi;
}

bool is_local_high(const std::vector<core::Bar>& bars, std::size_t center, int w) {
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

bool is_local_low(const std::vector<core::Bar>& bars, std::size_t center, int w) {
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

DivergenceSignal
DivergenceDetector::find(const std::vector<core::Bar>& bars) const {
    DivergenceSignal r;

    if (!validate()) return r;
    const std::size_t n = bars.size();
    if (n < static_cast<std::size_t>(opts_.rsi_period) + static_cast<std::size_t>(opts_.pivot_window) * 2 + 2)
        return r;

    const std::vector<double> rsi = compute_rsi(bars, opts_.rsi_period);

    const std::size_t cur = n - 1;
    if (rsi[cur] == 0.0) return r;

    const std::size_t pivot1_min = (cur > static_cast<std::size_t>(opts_.search_window))
        ? (cur - opts_.search_window) : 0;

    // Медвежья дивергенция.
    {
        std::size_t p2 = 0;
        bool p2_found = false;
        const std::size_t p2_min = (cur > static_cast<std::size_t>(opts_.pivot_window) * 2)
            ? (cur - opts_.pivot_window * 2) : 0;
        const std::size_t p2_max = (cur > static_cast<std::size_t>(opts_.pivot_window))
            ? (cur - opts_.pivot_window) : 0;

        // Поиск p2: от p2_max вниз до p2_min включительно.
        for (std::size_t i = p2_max; ; --i) {
            if (is_local_high(bars, i, opts_.pivot_window)) { p2 = i; p2_found = true; break; }
            if (i == p2_min) break;
        }

        if (p2_found) {
            // Поиск p1: от p2-1 вниз до pivot1_min включительно.
            for (std::size_t i = p2_min; ; ) {
                if (i == 0) break;
                --i;
                if (i >= p2) continue;
                if (i < pivot1_min) break;
                if (!is_local_high(bars, i, opts_.pivot_window)) continue;

                const double price1 = bars[i].high;
                const double price2 = bars[p2].high;
                const double rsi1 = rsi[i];
                const double rsi2 = rsi[p2];
                if (price2 > price1 + opts_.min_price_diff &&
                    rsi2 < rsi1 - opts_.min_rsi_diff) {
                    r.ok           = true;
                    r.direction    = core::OrderSide::Sell;
                    r.ts_pivot1    = bars[i].timestamp;
                    r.ts_pivot2    = bars[p2].timestamp;
                    r.price_pivot1 = price1;
                    r.price_pivot2 = price2;
                    r.rsi_pivot1   = rsi1;
                    r.rsi_pivot2   = rsi2;
                    r.rsi_current  = rsi[cur];
                    r.bars_between = static_cast<int>(p2 - i);
                    return r;
                }
            }
        }
    }

    // Бычья дивергенция.
    {
        std::size_t p2 = 0;
        bool p2_found = false;
        const std::size_t p2_min = (cur > static_cast<std::size_t>(opts_.pivot_window) * 2)
            ? (cur - opts_.pivot_window * 2) : 0;
        const std::size_t p2_max = (cur > static_cast<std::size_t>(opts_.pivot_window))
            ? (cur - opts_.pivot_window) : 0;

        for (std::size_t i = p2_max; ; --i) {
            if (is_local_low(bars, i, opts_.pivot_window)) { p2 = i; p2_found = true; break; }
            if (i == p2_min) break;
        }

        if (p2_found) {
            for (std::size_t i = p2_min; ; ) {
                if (i == 0) break;
                --i;
                if (i >= p2) continue;
                if (i < pivot1_min) break;
                if (!is_local_low(bars, i, opts_.pivot_window)) continue;

                const double price1 = bars[i].low;
                const double price2 = bars[p2].low;
                const double rsi1 = rsi[i];
                const double rsi2 = rsi[p2];
                if (price2 < price1 - opts_.min_price_diff &&
                    rsi2 > rsi1 + opts_.min_rsi_diff) {
                    r.ok           = true;
                    r.direction    = core::OrderSide::Buy;
                    r.ts_pivot1    = bars[i].timestamp;
                    r.ts_pivot2    = bars[p2].timestamp;
                    r.price_pivot1 = price1;
                    r.price_pivot2 = price2;
                    r.rsi_pivot1   = rsi1;
                    r.rsi_pivot2   = rsi2;
                    r.rsi_current  = rsi[cur];
                    r.bars_between = static_cast<int>(p2 - i);
                    return r;
                }
            }
        }
    }

    return r;
}

} // namespace spartak::cluster