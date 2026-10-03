// test_vlevel_detector.cpp
// Smoke-тест VLevelDetector.
#include "cluster/VLevelDetector.h"
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
// Восходящий импульс: n свечей, каждая +step.
static std::vector<core::Bar> make_up_impulse(int n, double start, double step) {
    std::vector<core::Bar> bars;
    int64_t ts = 1'700'000'000'000LL;
    double price = start;
    for (int i = 0; i < n; ++i) {
        core::Bar b;
        b.timestamp = ts + static_cast<int64_t>(i) * 300'000LL;
        b.open  = price;
        b.close = price + step;
        b.high  = b.close + 0.00001;
        b.low   = b.open  - 0.00001;
        b.ask_volume = 100;
        b.bid_volume = 100;
        bars.push_back(b);
        price += step;
    }
    return bars;
}
// Нисходящий импульс: n свечей, каждая -step.
static std::vector<core::Bar> make_dn_impulse(int n, double start, double step) {
    std::vector<core::Bar> bars;
    int64_t ts = 1'700'000'000'000LL;
    double price = start;
    for (int i = 0; i < n; ++i) {
        core::Bar b;
        b.timestamp = ts + static_cast<int64_t>(i) * 300'000LL;
        b.open  = price;
        b.close = price - step;
        b.high  = b.open  + 0.00001;
        b.low   = b.close - 0.00001;
        b.ask_volume = 100;
        b.bid_volume = 100;
        bars.push_back(b);
        price -= step;
    }
    return bars;
}
// Плоский поток (боковик).
static std::vector<core::Bar> make_flat(int n, double price) {
    std::vector<core::Bar> bars;
    int64_t ts = 1'700'000'000'000LL;
    for (int i = 0; i < n; ++i) {
        core::Bar b;
        b.timestamp = ts + static_cast<int64_t>(i) * 300'000LL;
        b.open = b.close = price;
        b.high = price + 0.00002;
        b.low  = price - 0.00002;
        b.ask_volume = 100;
        b.bid_volume = 100;
        bars.push_back(b);
    }
    return bars;
}
// Добавляет n нисходящих баров от последнего бара потока.
static void append_down(std::vector<core::Bar>& bars, int n, double step) {
    int64_t ts = bars.back().timestamp;
    double price = bars.back().close;
    for (int i = 0; i < n; ++i) {
        core::Bar b;
        b.timestamp = ts + static_cast<int64_t>(i + 1) * 300'000LL;
        b.open  = price;
        b.close = price - step;
        b.high  = b.open  + 0.00001;
        b.low   = b.close - 0.00001;
        b.ask_volume = 100;
        b.bid_volume = 100;
        bars.push_back(b);
        price -= step;
    }
}
int main() {
    // 1. Пустой вход
    {
        cluster::VLevelDetector d;
        const auto r = d.find({});
        check(!r.ok, "пустой вход -> ok=false");
    }
    // 2. Один бар
    {
        cluster::VLevelDetector d;
        std::vector<core::Bar> b(1);
        const auto r = d.find(b);
        check(!r.ok, "один бар -> ok=false");
    }
    // 3. Восходящий импульс 5 свечей
    {
        auto bars = make_up_impulse(5, 1.1000, 0.0020); // +1% за 5 свечей
        cluster::VLevelDetector d;
        const auto r = d.find(bars);
        check(r.ok, "восходящий импульс найден");
        check(r.direction == core::OrderSide::Buy, "направление Buy");
        check(r.bars_in_impulse >= 2 && r.bars_in_impulse <= 9, "длина 2-9");
        check(r.impulse_pct > 0.003, "сила > 0.3%");
        check(r.level_0 < r.level_50, "level_0 < level_50");
        check(r.level_50 < r.level_100, "level_50 < level_100");
        check(!r.is_broken, "импульс не перебит");
    }
    // 4. Нисходящий импульс 4 свечи
    {
        auto bars = make_dn_impulse(4, 1.1050, 0.0020);
        cluster::VLevelDetector d;
        const auto r = d.find(bars);
        check(r.ok, "нисходящий импульс найден");
        check(r.direction == core::OrderSide::Sell, "направление Sell");
        check(r.level_0 > r.level_100, "level_0 > level_100 для Sell");
    }
    // 5. Плоский поток — импульса нет
    {
        auto bars = make_flat(20, 1.1000);
        cluster::VLevelDetector d;
        const auto r = d.find(bars);
        check(!r.ok, "плоский поток -> ok=false");
    }
    // 6. Перебитие импульса: восходящий, потом короткий возврат ниже старта
    // Нисходящий намеренно короткий (3 бара), чтобы детектор выбрал восходящий.
    {
        auto bars = make_up_impulse(5, 1.1000, 0.0020); // 5 баров вверх
        append_down(bars, 3, 0.0050);                  // 3 бара вниз, резко
        cluster::VLevelDetector d;
        const auto r = d.find(bars);
        check(r.ok, "импульс найден");
        check(r.direction == core::OrderSide::Buy, "выбран восходящий импульс");
        check(r.is_broken, "импульс перебит после завершения");
    }
    // 7. find_last на длинном потоке
    {
        auto bars = make_up_impulse(80, 1.1000, 0.0005);
        cluster::VLevelDetector d;
        const auto r = d.find_last(bars);
        check(r.ok, "find_last -> ok=true");
    }
    std::cout << "Checks: " << g_checks << ", Failures: " << g_fails << "\n";
    return g_fails == 0 ? 0 : 1;
}