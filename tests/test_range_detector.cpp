// test_range_detector.cpp
// Smoke-тест RangeDetector.
#include "cluster/RangeDetector.h"
#include <iostream>
#include <vector>
using namespace spartak;
static int g_checks = 0;
static int g_fails  = 0;
static void check(bool cond, const char* msg) {
    ++g_checks;
    if (!cond) {
        ++g_fails;
        std::cout << "FAIL: " << msg << "\n";
    }
}
// Строит боковик: n баров, цена ходит lo-mid-hi-mid.
static std::vector<core::Bar> make_range(int n, double lo, double hi) {
    std::vector<core::Bar> bars;
    bars.reserve(static_cast<std::size_t>(n));
    int64_t ts = 1'700'000'000'000LL;
    const double mid = (lo + hi) * 0.5;
    const double prices[6] = {lo, lo, mid, hi, hi, mid};
    for (int i = 0; i < n; ++i) {
        const int k = i % 6;
        core::Bar b;
        b.timestamp = ts + static_cast<int64_t>(i) * 300'000LL;
        b.open = b.close = prices[k];
        b.high = (k == 3 || k == 4) ? hi : prices[k] + 0.00005;
        b.low  = (k == 0 || k == 1) ? lo : prices[k] - 0.00005;
        b.ask_volume = 100;
        b.bid_volume = 100;
        bars.push_back(b);
    }
    return bars;
}
// Строит ровный восходящий тренд без боковика.
static std::vector<core::Bar> make_trend(int n, double start, double step) {
    std::vector<core::Bar> bars;
    bars.reserve(static_cast<std::size_t>(n));
    int64_t ts = 1'700'000'000'000LL;
    for (int i = 0; i < n; ++i) {
        core::Bar b;
        b.timestamp = ts + static_cast<int64_t>(i) * 300'000LL;
        b.open = b.close = start + i * step;
        b.high = b.close + 0.0001;
        b.low  = b.close - 0.0001;
        b.ask_volume = 100;
        b.bid_volume = 100;
        bars.push_back(b);
    }
    return bars;
}
int main() {
    // 1. Пустой вход
    {
        cluster::RangeDetector d;
        const auto r = d.find({});
        check(!r.ok, "пустой вход -> ok=false");
    }
    // 2. Один бар
    {
        cluster::RangeDetector d;
        std::vector<core::Bar> b(1);
        const auto r = d.find(b);
        check(!r.ok, "один бар -> ok=false");
    }
    // 3. Явный боковик
    {
        const auto bars = make_range(30, 1.1000, 1.1050);
        cluster::RangeDetector d;
        const auto r = d.find(bars);
        check(r.ok, "боковик найден");
        check(r.touches_top    >= 2, "касаний верха >= 2");
        check(r.touches_bottom >= 2, "касаний низа >= 2");
        check(r.width > 0.0, "ширина > 0");
        check(r.potential > r.width, "потенциал > ширина");
        check(r.ask_sum == r.bid_sum, "ASK == BID при равных объёмах");
    }
    // 4. Ровный тренд — боковика нет
    {
        const auto bars = make_trend(30, 1.1000, 0.005);
        cluster::RangeDetector d;
        const auto r = d.find(bars);
        check(!r.ok, "тренд -> ok=false");
    }
    // 5. Покупатели доминируют — ждём вниз
    {
        auto bars = make_range(30, 1.1000, 1.1050);
        for (auto& b : bars) {
            b.ask_volume = 300;
            b.bid_volume = 100;
        }
        cluster::RangeDetector d;
        const auto r = d.find(bars);
        check(r.ok, "боковик найден (для теста направления)");
        check(r.expected_breakout == core::OrderSide::Sell,
              "покупателей больше -> ждём вниз");
    }
    // 6. Продавцы доминируют — ждём вверх
    {
        auto bars = make_range(30, 1.1000, 1.1050);
        for (auto& b : bars) {
            b.ask_volume = 100;
            b.bid_volume = 300;
        }
        cluster::RangeDetector d;
        const auto r = d.find(bars);
        check(r.ok, "боковик найден (для теста направления)");
        check(r.expected_breakout == core::OrderSide::Buy,
              "продавцов больше -> ждём вверх");
    }
    // 7. find_last на длинном потоке
    {
        auto bars = make_range(100, 1.1000, 1.1050);
        cluster::RangeDetector d;
        const auto r = d.find_last(bars);
        check(r.ok, "find_last -> ok=true");
    }
    std::cout << "Checks: " << g_checks << ", Failures: " << g_fails << "\n";
    return g_fails == 0 ? 0 : 1;
}