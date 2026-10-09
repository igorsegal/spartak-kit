// test_stop_hunt.cpp
#include "cluster/StopHuntDetector.h"
#include <algorithm>
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

static void add_bar(std::vector<core::Bar>& bars, int64_t& ts,
                    double o, double h, double l, double c) {
    core::Bar b;
    b.timestamp = ts;
    b.open = o; b.high = h; b.low = l; b.close = c;
    b.ask_volume = 100; b.bid_volume = 100;
    b.volume = 200;
    b.delta = 0;
    bars.push_back(b);
    ts += 300'000LL;
}

// Снятие стопов продавцов: прокол вверх уровня 1.1050, возврат, уход вниз.
static std::vector<core::Bar> make_hunt_up() {
    std::vector<core::Bar> bars;
    int64_t ts = 1'700'000'000'000LL;

    // Спокойные бары вокруг 1.1000.
    for (int i = 0; i < 30; ++i) {
        double p = 1.1000 + ((i % 2 == 0) ? 0.0002 : -0.0002);
        add_bar(bars, ts, 1.1000, p + 0.0003, p - 0.0003, p);
    }

    // Касание 1.1050 сверху.
    add_bar(bars, ts, 1.1030, 1.1050, 1.1025, 1.1040);
    add_bar(bars, ts, 1.1040, 1.1045, 1.1030, 1.1035);

    // Второе касание 1.1050.
    add_bar(bars, ts, 1.1035, 1.1050, 1.1030, 1.1044);
    add_bar(bars, ts, 1.1044, 1.1047, 1.1035, 1.1040);

    // Прокол вверх: close выше 1.1050 + tol.
    add_bar(bars, ts, 1.1040, 1.1070, 1.1040, 1.1065);

    // Возврат: close в зоне уровня.
    add_bar(bars, ts, 1.1065, 1.1070, 1.1030, 1.1045);

    // Уход вниз.
    add_bar(bars, ts, 1.1045, 1.1050, 1.1020, 1.1025);
    add_bar(bars, ts, 1.1025, 1.1030, 1.1000, 1.1005);
    add_bar(bars, ts, 1.1005, 1.1010, 1.0990, 1.0995);

    return bars;
}

// Снятие стопов покупателей: прокол вниз уровня 1.0950, возврат, уход вверх.
static std::vector<core::Bar> make_hunt_down() {
    std::vector<core::Bar> bars;
    int64_t ts = 1'700'000'000'000LL;

    for (int i = 0; i < 30; ++i) {
        double p = 1.1000 + ((i % 2 == 0) ? 0.0002 : -0.0002);
        add_bar(bars, ts, 1.1000, p + 0.0003, p - 0.0003, p);
    }

    // Касание 1.0950 снизу.
    add_bar(bars, ts, 1.0970, 1.0975, 1.0950, 1.0960);
    add_bar(bars, ts, 1.0960, 1.0965, 1.0955, 1.0965);

    // Второе касание 1.0950.
    add_bar(bars, ts, 1.0965, 1.0970, 1.0950, 1.0955);
    add_bar(bars, ts, 1.0955, 1.0965, 1.0955, 1.0960);

    // Прокол вниз: close ниже 1.0950 - tol.
    add_bar(bars, ts, 1.0960, 1.0960, 1.0930, 1.0935);

    // Возврат в зону.
    add_bar(bars, ts, 1.0935, 1.0970, 1.0930, 1.0955);

    // Уход вверх.
    add_bar(bars, ts, 1.0955, 1.0990, 1.0955, 1.0985);
    add_bar(bars, ts, 1.0985, 1.1010, 1.0980, 1.1005);
    add_bar(bars, ts, 1.1005, 1.1030, 1.1000, 1.1025);

    return bars;
}

int main() {
    // 1. Пустой вход.
    {
        cluster::StopHuntDetector d;
        const auto r = d.find({});
        check(!r.ok, "empty input -> ok=false");
    }

    // 2. Слишком мало баров.
    {
        std::vector<core::Bar> bars;
        int64_t ts = 1'700'000'000'000LL;
        for (int i = 0; i < 10; ++i) {
            add_bar(bars, ts, 1.1, 1.1005, 1.0995, 1.1001);
        }
        cluster::StopHuntDetector d;
        const auto r = d.find(bars);
        check(!r.ok, "too few bars -> ok=false");
    }

    // 3. Плоский ряд.
    {
        std::vector<core::Bar> bars;
        int64_t ts = 1'700'000'000'000LL;
        for (int i = 0; i < 30; ++i) {
            add_bar(bars, ts, 1.1000, 1.1000, 1.1000, 1.1000);
        }
        cluster::StopHuntDetector d;
        const auto r = d.find(bars);
        check(!r.ok, "flat range -> ok=false");
    }

    // 4. Снятие стопов продавцов (прокол вверх) -> Sell.
    {
        auto bars = make_hunt_up();
        cluster::StopHuntDetector d;
        const auto r = d.find(bars);
        check(r.ok, "hunt up found");
        if (r.ok) {
            check(r.direction == core::OrderSide::Sell, "hunt up -> Sell");
            check(r.touches_before >= 2, "touches_before >= 2");
            check(r.pierce_extreme > r.level, "pierce_extreme > level (up)");
            check(r.bars_outside > 0, "bars_outside > 0");
        }
    }

    // 5. Снятие стопов покупателей (прокол вниз) -> Buy.
    {
        auto bars = make_hunt_down();
        cluster::StopHuntDetector d;
        const auto r = d.find(bars);
        check(r.ok, "hunt down found");
        if (r.ok) {
            check(r.direction == core::OrderSide::Buy, "hunt down -> Buy");
            check(r.touches_before >= 2, "touches_before >= 2");
            check(r.pierce_extreme < r.level, "pierce_extreme < level (down)");
        }
    }

    // 6. Невалидные параметры.
    {
        cluster::StopHuntOptions opts;
        opts.max_pierce_bars = 0;
        cluster::StopHuntDetector d(opts);
        auto bars = make_hunt_up();
        const auto r = d.find(bars);
        check(!r.ok, "invalid max_pierce_bars -> ok=false");
    }

    // 7. Монотонный тренд без уровней с проколом.
    {
        std::vector<core::Bar> bars;
        int64_t ts = 1'700'000'000'000LL;
        double p = 1.1000;
        for (int i = 0; i < 80; ++i) {
            const double o = p;
            const double c = p + 0.0005;
            add_bar(bars, ts, o, c + 0.0002, o - 0.0002, c);
            p = c;
        }
        cluster::StopHuntDetector d;
        const auto r = d.find(bars);
        // Может найти ложный уровень, но не должен падать.
        check(true, "monotonic doesn't crash");
    }

    std::cout << "Checks: " << g_checks << ", Failures: " << g_fails << "\n";
    return g_fails == 0 ? 0 : 1;
}