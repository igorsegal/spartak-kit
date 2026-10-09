// test_cascade_level.cpp
#include "cluster/CascadeLevelDetector.h"
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

// Каскад: два уровня 1.1050 (сверху) и 1.0950 (снизу). Отбой от обоих.
static std::vector<core::Bar> make_cascade() {
    std::vector<core::Bar> bars;
    int64_t ts = 1'700'000'000'000LL;

    // Спокойные бары вокруг 1.1000.
    for (int i = 0; i < 20; ++i) {
        double p = 1.1000 + ((i % 2 == 0) ? 0.0002 : -0.0002);
        add_bar(bars, ts, 1.1000, p + 0.0003, p - 0.0003, p);
    }

    // Касание 1.1050 сверху.
    add_bar(bars, ts, 1.1030, 1.1050, 1.1025, 1.1040);
    // Отбой от 1.1050 вниз.
    add_bar(bars, ts, 1.1040, 1.1045, 1.1010, 1.1015);

    // Касание 1.0950 снизу.
    add_bar(bars, ts, 1.0970, 1.0975, 1.0950, 1.0960);
    // Отбой от 1.0950 вверх.
    add_bar(bars, ts, 1.0960, 1.0995, 1.0955, 1.0990);

    // Спокойные бары после.
    for (int i = 0; i < 10; ++i) {
        double p = 1.1000 + ((i % 2 == 0) ? 0.0002 : -0.0002);
        add_bar(bars, ts, 1.1000, p + 0.0003, p - 0.0003, p);
    }

    return bars;
}

int main() {
    // 1. Пустой вход.
    {
        cluster::CascadeLevelDetector d;
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
        cluster::CascadeLevelDetector d;
        const auto r = d.find(bars);
        check(!r.ok, "too few bars -> ok=false");
    }

    // 3. Плоский ряд (диапазон = 0).
    {
        std::vector<core::Bar> bars;
        int64_t ts = 1'700'000'000'000LL;
        for (int i = 0; i < 30; ++i) {
            add_bar(bars, ts, 1.1000, 1.1000, 1.1000, 1.1000);
        }
        cluster::CascadeLevelDetector d;
        const auto r = d.find(bars);
        check(!r.ok, "flat range -> ok=false");
    }

    // 4. Каскад найден.
    {
        auto bars = make_cascade();
        cluster::CascadeLevelDetector d;
        const auto r = d.find(bars);
        check(r.ok, "cascade found");
        if (r.ok) {
            check(r.direction == core::OrderSide::Sell, "direction Sell (A > B)");
            check(r.level_a > r.level_b, "level_a > level_b");
            check(r.touches_a >= 1, "touches_a >= 1");
            check(r.touches_b >= 1, "touches_b >= 1");
            check(r.bars_between >= 0, "bars_between >= 0");
            check(r.level_a_ts > 0, "level_a_ts > 0");
            check(r.level_b_ts > 0, "level_b_ts > 0");
        }
    }

    // 5. Только один уровень - не пара.
    {
        std::vector<core::Bar> bars;
        int64_t ts = 1'700'000'000'000LL;
        for (int i = 0; i < 20; ++i) {
            double p = 1.1000 + ((i % 2 == 0) ? 0.0002 : -0.0002);
            add_bar(bars, ts, 1.1000, p + 0.0003, p - 0.0003, p);
        }
        add_bar(bars, ts, 1.1030, 1.1050, 1.1025, 1.1040);
        add_bar(bars, ts, 1.1040, 1.1045, 1.1010, 1.1015);
        for (int i = 0; i < 10; ++i) {
            double p = 1.1000 + ((i % 2 == 0) ? 0.0002 : -0.0002);
            add_bar(bars, ts, 1.1000, p + 0.0003, p - 0.0003, p);
        }
        cluster::CascadeLevelDetector d;
        const auto r = d.find(bars);
        check(!r.ok, "one level -> ok=false");
    }

    // 6. Параметры не прошли валидацию.
    {
        cluster::CascadeOptions opts;
        opts.zone_tolerance_range = 2.0;
        cluster::CascadeLevelDetector d(opts);
        auto bars = make_cascade();
        const auto r = d.find(bars);
        check(!r.ok, "invalid opts -> ok=false");
    }

    std::cout << "Checks: " << g_checks << ", Failures: " << g_fails << "\n";
    return g_fails == 0 ? 0 : 1;
}