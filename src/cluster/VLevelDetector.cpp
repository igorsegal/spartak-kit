#include "cluster/VLevelDetector.h"
#include <algorithm>
#include <cmath>
#include <cstddef>
namespace spartak::cluster {
namespace {
// Возвращает длину импульса в барах, начиная с индекса start.
// 0 — импульса нет.
// direction: +1 вверх, -1 вниз.
int detect_impulse_length(const std::vector<core::Bar>& bars,
                          std::size_t start,
                          int max_len,
                          int min_len,
                          double one_way_ratio)
{
    const std::size_t n = bars.size();
    if (start >= n) return 0;
    const int maxk = static_cast<int>(std::min<std::size_t>(
        static_cast<std::size_t>(max_len), n - start));
    int best_len = 0;
    for (int len = min_len; len <= maxk; ++len) {
        int up = 0;
        int down = 0;
        for (int k = 0; k < len; ++k) {
            const auto& b = bars[start + static_cast<std::size_t>(k)];
            if (b.close > b.open) ++up;
            else if (b.close < b.open) ++down;
        }
        const double ratio = static_cast<double>(std::max(up, down)) / len;
        if (ratio >= one_way_ratio) {
            best_len = len;
        }
    }
    return best_len;
}
// Знак импульса: +1 вверх, -1 вниз.
int impulse_direction(const std::vector<core::Bar>& bars,
                      std::size_t start, int len)
{
    double sum_up = 0.0;
    double sum_dn = 0.0;
    for (int k = 0; k < len; ++k) {
        const auto& b = bars[start + static_cast<std::size_t>(k)];
        const double body = b.close - b.open;
        if (body > 0.0) sum_up += body;
        else if (body < 0.0) sum_dn += -body;
    }
    return (sum_up >= sum_dn) ? +1 : -1;
}
} // namespace
VLevelSignal
VLevelDetector::find(const std::vector<core::Bar>& bars) const {
    VLevelSignal r;
    if (bars.size() < static_cast<std::size_t>(opts_.min_impulse_bars) + 1) return r;
    // 1. Перебираем все стартовые точки, ищем лучший импульс.
    // Лучший — самый длинный и самый сильный.
    std::size_t best_start = 0;
    int         best_len   = 0;
    int         best_dir   = 0;
    double      best_pct   = 0.0;
    for (std::size_t i = 0; i + static_cast<std::size_t>(opts_.min_impulse_bars) <= bars.size(); ++i) {
        const int len = detect_impulse_length(bars, i,
                                              opts_.max_impulse_bars,
                                              opts_.min_impulse_bars,
                                              opts_.one_way_ratio);
        if (len < opts_.min_impulse_bars) continue;
        const int dir = impulse_direction(bars, i, len);
        const std::size_t s = i;
        const std::size_t e = i + static_cast<std::size_t>(len) - 1;
        const double start_price = bars[s].open;
        const double end_price   = bars[e].close;
        const double avg         = (start_price + end_price) * 0.5;
        if (avg <= 0.0) continue;
        const double pct = std::fabs(end_price - start_price) / avg;
        if (pct < opts_.min_strength) continue;
        // Лучший импульс: длиннее, при равной длине — сильнее.
        if (len > best_len || (len == best_len && pct > best_pct)) {
            best_start = s;
            best_len   = len;
            best_dir   = dir;
            best_pct   = pct;
        }
    }
    if (best_len < opts_.min_impulse_bars) return r;
    // 2. Заполнить результат
    const std::size_t s = best_start;
    const std::size_t e = best_start + static_cast<std::size_t>(best_len) - 1;
    r.ok              = true;
    r.direction       = (best_dir > 0) ? core::OrderSide::Buy : core::OrderSide::Sell;
    r.start_ts        = bars[s].timestamp;
    r.end_ts          = bars[e].timestamp;
    r.start_price     = bars[s].open;
    r.end_price       = bars[e].close;
    r.level_0         = r.start_price;
    r.level_50        = (r.start_price + r.end_price) * 0.5;
    r.level_100       = r.end_price;
    r.bars_in_impulse = best_len;
    r.impulse_pct     = best_pct;
    // 3. Проверить перебитие импульса после его завершения
    // Вверх: если цена после e ушла ниже start_price — перебит.
    // Вниз:  если цена после e ушла выше start_price — перебит.
    for (std::size_t i = e + 1; i < bars.size(); ++i) {
        if (best_dir > 0 && bars[i].low <= r.start_price) { r.is_broken = true; break; }
        if (best_dir < 0 && bars[i].high >= r.start_price) { r.is_broken = true; break; }
    }
    // 4. Где цена сейчас — у 50% или у 100%
    const auto& last_bar = bars.back();
    const double tol50  = r.level_50  * opts_.zone_tolerance;
    const double tol100 = r.level_100 * opts_.zone_tolerance;
    const bool near50 = (std::fabs(last_bar.close - r.level_50) <= tol50)
                     || (last_bar.low <= r.level_50 + tol50 && last_bar.high >= r.level_50 - tol50);
    const bool near100 = (std::fabs(last_bar.close - r.level_100) <= tol100)
                      || (last_bar.low <= r.level_100 + tol100 && last_bar.high >= r.level_100 - tol100);
    r.in_zone_50  = near50;
    r.in_zone_100 = near100;
    return r;
}
VLevelSignal
VLevelDetector::find_last(const std::vector<core::Bar>& bars) const {
    if (bars.size() <= opts_.window_size) {
        return find(bars);
    }
    const std::size_t start = bars.size() - opts_.window_size;
    std::vector<core::Bar> window(bars.begin() + start, bars.end());
    return find(window);
}
} // namespace spartak::cluster