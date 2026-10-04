#include "cluster/VLevelDetector.h"
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <vector>

namespace spartak::cluster {
namespace {

// Знак импульса: +1 вверх, -1 вниз, 0 — нет движения.
// end_exclusive = start + len — эксклюзивный индекс в префиксных массивах.
int impulse_direction(const std::vector<double>& body_up,
                      const std::vector<double>& body_dn,
                      std::size_t start, int len)
{
    if (len <= 0) return 0;
    const std::size_t end_exclusive = start + static_cast<std::size_t>(len);
    if (end_exclusive >= body_up.size() || end_exclusive >= body_dn.size()) return 0;
    const double up = body_up[end_exclusive] - body_up[start];
    const double dn = body_dn[end_exclusive] - body_dn[start];
    if (up == 0.0 && dn == 0.0) return 0;
    return (up >= dn) ? +1 : -1;
}

} // namespace

VLevelSignal
VLevelDetector::find(const std::vector<core::Bar>& bars) const {
    return find_impl(bars, 0, bars.size());
}

VLevelSignal
VLevelDetector::find_last(const std::vector<core::Bar>& bars) const {
    if (bars.size() <= static_cast<std::size_t>(opts_.window_size)) {
        return find_impl(bars, 0, bars.size());
    }
    const std::size_t start = bars.size() - static_cast<std::size_t>(opts_.window_size);
    return find_impl(bars, start, bars.size());
}

VLevelSignal
VLevelDetector::find_impl(const std::vector<core::Bar>& bars,
                          std::size_t search_begin,
                          std::size_t search_end) const
{
    VLevelSignal r;

    if (opts_.min_impulse_bars < 1) return r;
    if (opts_.max_impulse_bars < opts_.min_impulse_bars) return r;
    if (opts_.one_way_ratio <= 0.0 || opts_.one_way_ratio > 1.0) return r;
    if (opts_.zone_tolerance <= 0.0 || opts_.zone_tolerance >= 1.0) return r;
    if (opts_.window_size < opts_.min_impulse_bars + 1) return r;

    const std::size_t n = bars.size();
    if (n < static_cast<std::size_t>(opts_.min_impulse_bars) + 1) return r;
    if (search_end > n) search_end = n;
    if (search_begin >= search_end) return r;

    // Префиксные массивы строятся только до search_end — для find_last это O(window_size).
    // Индексация: up_bars[k] — суммарные данные по барам [0, k).
    const std::size_t prefix_size = search_end + 1;
    std::vector<int>    up_bars(prefix_size, 0);
    std::vector<int>    dn_bars(prefix_size, 0);
    std::vector<double> body_up(prefix_size, 0.0);
    std::vector<double> body_dn(prefix_size, 0.0);
    for (std::size_t i = 0; i < search_end; ++i) {
        const auto& b = bars[i];
        up_bars[i + 1] = up_bars[i];
        dn_bars[i + 1] = dn_bars[i];
        body_up[i + 1] = body_up[i];
        body_dn[i + 1] = body_dn[i];
        const double body = b.close - b.open;
        if (body > 0.0) { ++up_bars[i + 1]; body_up[i + 1] += body; }
        else if (body < 0.0) { ++dn_bars[i + 1]; body_dn[i + 1] += -body; }
    }

    const int min_len = opts_.min_impulse_bars;
    const int max_len = opts_.max_impulse_bars;

    std::size_t best_start = 0;
    int         best_len   = 0;
    int         best_dir   = 0;
    double      best_pct   = 0.0;

    for (std::size_t i = search_begin;
         i + static_cast<std::size_t>(min_len) <= search_end;
         ++i)
    {
        const int maxk = static_cast<int>(std::min<std::size_t>(
            static_cast<std::size_t>(max_len), search_end - i));

        // Правило выбора длины: самый длинный импульс, прошедший все фильтры.
        // При равной длине — сильнее (по pct).
        int    local_len = 0;
        int    local_dir = 0;
        double local_pct = 0.0;

        for (int len = min_len; len <= maxk; ++len) {
            const int up = up_bars[i + static_cast<std::size_t>(len)] - up_bars[i];
            const int dn = dn_bars[i + static_cast<std::size_t>(len)] - dn_bars[i];
            const double ratio = static_cast<double>(std::max(up, dn)) / len;
            if (ratio < opts_.one_way_ratio) continue;

            const int dir = impulse_direction(body_up, body_dn, i, len);
            if (dir == 0) continue;

            const std::size_t s = i;
            const std::size_t e = i + static_cast<std::size_t>(len) - 1;
            const double start_price = bars[s].open;
            const double end_price   = bars[e].close;
            const double avg         = (start_price + end_price) * 0.5;
            if (avg == 0.0) continue;
            const double pct = std::fabs(end_price - start_price) / std::fabs(avg);
            if (pct < opts_.min_strength) continue;

            if (len > local_len || (len == local_len && pct > local_pct)) {
                local_len = len;
                local_dir = dir;
                local_pct = pct;
            }
        }

        if (local_len < min_len || local_dir == 0) continue;

        if (local_len > best_len || (local_len == best_len && local_pct > best_pct)) {
            best_start = i;
            best_len   = local_len;
            best_dir   = local_dir;
            best_pct   = local_pct;
        }
    }

    if (best_len < min_len || best_dir == 0) return r;

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

    // Перебитие: проверяются бары строго после e.
    // Внутри импульса перебитие не считается — это определение импульса.
    for (std::size_t i = e + 1; i < n; ++i) {
        if (best_dir > 0 && bars[i].low  <= r.start_price) { r.is_broken = true; break; }
        if (best_dir < 0 && bars[i].high >= r.start_price) { r.is_broken = true; break; }
    }

    // Зоны у 50% и 100% определяются по последнему бару всего массива,
    // то есть показывают, где цена находится сейчас, а не в момент импульса.
    // Толеранс — доля от |уровня|; abs корректно работает с отрицательными ценами.
    const auto& last_bar = bars.back();
    const double tol50  = std::fabs(r.level_50)  * opts_.zone_tolerance;
    const double tol100 = std::fabs(r.level_100) * opts_.zone_tolerance;
    r.in_zone_50 = (std::fabs(last_bar.close - r.level_50) <= tol50)
                || (last_bar.low <= r.level_50 + tol50 && last_bar.high >= r.level_50 - tol50);
    r.in_zone_100 = (std::fabs(last_bar.close - r.level_100) <= tol100)
                 || (last_bar.low <= r.level_100 + tol100 && last_bar.high >= r.level_100 - tol100);

    return r;
}

} // namespace spartak::cluster