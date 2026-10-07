// test_false_breakout_detector.cpp
#include "cluster/FalseBreakoutDetector.h"
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
    bars.push_back(b);
    ts += 300'000LL;
}

static std::vector<core::Bar> make_false_up() {
    std::vector<core::Bar> bars;
    int64_t ts = 1'700'000'000'000LL;

    for (int i = 0; i < 40; ++i) {
        double p = 1.1000 + ((i % 2 == 0) ? 0.0002 : -0.0002);
        add_bar(bars, ts, 1.1000, p + 0.0005, p - 0.0005, p);
    }

    add_bar(bars, ts, 1.1030, 1.1050, 1.1025, 1.1040);
    add_bar(bars, ts, 1.1040, 1.1045, 1.1030, 1.1035);
    add_bar(bars, ts, 1.1035, 1.1048, 1.1030, 1.1042);
    add_bar(bars, ts, 1.1042, 1.1050, 1.1035, 1.1044);
    add_bar(bars, ts, 1.1044, 1.1046, 1.1035, 1.1038);
    add_bar(bars, ts, 1.1038, 1.1049, 1.1032, 1.1044);
    add_bar(bars, ts, 1.1044, 1.1050, 1.1038, 1.1045);
    add_bar(bars, ts, 1.1045, 1.1047, 1.1035, 1.1040);

    add_bar(bars, ts, 1.1040, 1.1085, 1.1040, 1.1080);
    add_bar(bars, ts, 1.1080, 1.1090, 1.1075, 1.1085);
    add_bar(bars, ts, 1.1085, 1.1092, 1.1080, 1.1088);

    add_bar(bars, ts, 1.1088, 1.1088, 1.1040, 1.1045);

    add_bar(bars, ts, 1.1045, 1.1050, 1.1020, 1.1025);
    add_bar(bars, ts, 1.1025, 1.1030, 1.1000, 1.1005);
    add_bar(bars, ts, 1.1005, 1.1010, 1.0990, 1.0995);
    add_bar(bars, ts, 1.0995, 1.1000, 1.0980, 1.0985);
    add_bar(bars, ts, 1.0985, 1.0990, 1.0970, 1.0975);

    return bars;
}

static std::vector<core::Bar> make_false_down() {
    std::vector<core::Bar> bars;
    int64_t ts = 1'700'000'000'000LL;

    for (int i = 0; i < 40; ++i) {
        double p = 1.2000 + ((i % 2 == 0) ? 0.0002 : -0.0002);
        add_bar(bars, ts, 1.2000, p + 0.0005, p - 0.0005, p);
    }

    add_bar(bars, ts, 1.1970, 1.1975, 1.1950, 1.1960);
    add_bar(bars, ts, 1.1960, 1.1965, 1.1955, 1.1965);
    add_bar(bars, ts, 1.1965, 1.1970, 1.1952, 1.1960);
    add_bar(bars, ts, 1.1960, 1.1965, 1.1950, 1.1955);
    add_bar(bars, ts, 1.1955, 1.1965, 1.1953, 1.1960);
    add_bar(bars, ts, 1.1960, 1.1965, 1.1951, 1.1958);
    add_bar(bars, ts, 1.1958, 1.1962, 1.1950, 1.1955);
    add_bar(bars, ts, 1.1955, 1.1965, 1.1954, 1.1960);

    add_bar(bars, ts, 1.1960, 1.1960, 1.1915, 1.1920);
    add_bar(bars, ts, 1.1920, 1.1925, 1.1910, 1.1915);
    add_bar(bars, ts, 1.1915, 1.1920, 1.1908, 1.1912);

    add_bar(bars, ts, 1.1912, 1.1960, 1.1912, 1.1955);

    add_bar(bars, ts, 1.1955, 1.1980, 1.1950, 1.1975);
    add_bar(bars, ts, 1.1975, 1.2000, 1.1970, 1.1995);
    add_bar(bars, ts, 1.1995, 1.2020, 1.1990, 1.2015);
    add_bar(bars, ts, 1.2015, 1.2040, 1.2010, 1.2035);
    add_bar(bars, ts, 1.2035, 1.2060, 1.2030, 1.2055);

    return bars;
}

int main() {
    {
        cluster::FalseBreakoutDetector d;
        const auto r = d.find({});
        check(!r.ok, "empty input -> ok=false");
    }

    {
        std::vector<core::Bar> bars;
        int64_t ts = 1'700'000'000'000LL;
        for (int i = 0; i < 30; ++i) {
            add_bar(bars, ts, 1.1, 1.1005, 1.0995, 1.1001);
        }
        cluster::FalseBreakoutDetector d;
        const auto r = d.find(bars);
        check(!r.ok, "too few bars -> ok=false");
    }

    {
        auto bars = make_false_up();
        cluster::FalseBreakoutDetector d;
        const auto r = d.find(bars);
        check(r.ok, "false breakout up found");
        if (r.ok) {
            check(r.direction == core::OrderSide::Sell, "direction Sell");
            check(r.touches_before >= 3, "touches >= 3");
            check(r.bars_outside > 0, "bars_outside > 0");
            check(r.breakout_extreme > r.level, "extreme > level (up)");
        }
    }

    {
        auto bars = make_false_down();
        cluster::FalseBreakoutDetector d;
        const auto r = d.find(bars);
        check(r.ok, "false breakout down found");
        if (r.ok) {
            check(r.direction == core::OrderSide::Buy, "direction Buy");
            check(r.touches_before >= 3, "touches >= 3");
            check(r.breakout_extreme < r.level, "extreme < level (down)");
        }
    }

    {
        std::vector<core::Bar> bars;
        int64_t ts = 1'700'000'000'000LL;
        double p = 1.1000;
        for (int i = 0; i < 100; ++i) {
            const double o = p;
            const double c = p + 0.0005;
            add_bar(bars, ts, o, c + 0.0002, o - 0.0002, c);
            p = c;
        }
        cluster::FalseBreakoutDetector d;
        const auto r = d.find(bars);
        check(!r.ok || r.touches_before >= 3, "monotonic: no false signals");
    }

    std::cout << "Checks: " << g_checks << ", Failures: " << g_fails << "\n";
    return g_fails == 0 ? 0 : 1;
}