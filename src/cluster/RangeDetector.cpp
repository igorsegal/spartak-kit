#include "cluster/RangeDetector.h"
#include <algorithm>
#include <cstddef>
namespace spartak::cluster {
namespace {
struct TouchCounts {
    int top = 0;
    int bottom = 0;
};
TouchCounts count_touches(const std::vector<core::Bar>& bars,
                          double hi, double lo, double tol)
{
    const double thr_top = hi * (1.0 - tol);
    const double thr_bot = lo * (1.0 + tol);
    TouchCounts tc;
    for (const auto& b : bars) {
        if (b.high >= thr_top) ++tc.top;
        if (b.low  <= thr_bot) ++tc.bottom;
    }
    return tc;
}
// Проверка: касания должны быть разбросаны по окну.
// Иначе это тренд, а не боковик.
bool touches_are_spread(const std::vector<core::Bar>& bars,
                        double hi, double lo, double tol,
                        bool check_top)
{
    const std::size_t n = bars.size();
    if (n == 0) return false;
    const double thr_top = hi * (1.0 - tol);
    const double thr_bot = lo * (1.0 + tol);
    std::size_t first = n;
    std::size_t last  = 0;
    bool found = false;
    for (std::size_t i = 0; i < n; ++i) {
        const bool hit = check_top
            ? (bars[i].high >= thr_top)
            : (bars[i].low  <= thr_bot);
        if (hit) {
            if (!found) { first = i; found = true; }
            last = i;
        }
    }
    if (!found) return false;
    const std::size_t quarter = n / 4;
    return (last - first) >= quarter;
}
} // namespace
RangeSignal
RangeDetector::find(const std::vector<core::Bar>& bars) const {
    RangeSignal r;
    if (bars.size() < 2) return r;
    // 1. Границы
    double hi = bars[0].high;
    double lo = bars[0].low;
    for (const auto& b : bars) {
        if (b.high > hi) hi = b.high;
        if (b.low  < lo) lo = b.low;
    }
    // 2. Ширина
    const double width = hi - lo;
    if (width <= 0.0) return r;
    // 3. Минимальная ширина (доля от средней цены)
    const double avg_price = (hi + lo) * 0.5;
    if (avg_price <= 0.0) return r;
    if ((width / avg_price) < opts_.min_width) return r;
    // 4. Касания
    const TouchCounts tc = count_touches(bars, hi, lo, opts_.touch_tolerance);
    if (tc.top    < opts_.min_touches_top)    return r;
    if (tc.bottom < opts_.min_touches_bottom) return r;
    // 5. Разброс касаний
    if (!touches_are_spread(bars, hi, lo, opts_.touch_tolerance, true))  return r;
    if (!touches_are_spread(bars, hi, lo, opts_.touch_tolerance, false)) return r;
    // 6. Заполнить результат
    r.ok             = true;
    r.start_ts       = bars.front().timestamp;
    r.end_ts         = bars.back().timestamp;
    r.high           = hi;
    r.low            = lo;
    r.width          = width;
    r.potential      = width * 2.0;
    r.touches_top    = tc.top;
    r.touches_bottom = tc.bottom;
    // 7. Суммы ASK / BID
    for (const auto& b : bars) {
        r.ask_sum += b.ask_volume;
        r.bid_sum += b.bid_volume;
    }
    // 8. Ожидаемое направление прорыва
    // Правило: покупателей больше — ждём вниз; продавцов больше — ждём вверх.
    r.expected_breakout = (r.ask_sum > r.bid_sum)
        ? core::OrderSide::Sell
        : core::OrderSide::Buy;
    return r;
}
RangeSignal
RangeDetector::find_last(const std::vector<core::Bar>& bars) const {
    if (bars.size() <= opts_.window_size) {
        return find(bars);
    }
    const std::size_t start = bars.size() - opts_.window_size;
    std::vector<core::Bar> window(bars.begin() + start, bars.end());
    return find(window);
}
} // namespace spartak::cluster