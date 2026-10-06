// test_mirror_detector.cpp
#include "cluster/MirrorLevelDetector.h"
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

// Уровень сверху, 3 касания, пробой вверх, ретест, отбой.
static std::vector<core::Bar> make_mirror_up() {
    std::vector<core::Bar> bars;
    int64_t ts = 1'700'000'000'000LL;
    add_bar(bars, ts, 1.1000, 1.1010, 1.0995, 1.1005);
    add_bar(bars, ts, 1.1005, 1.1020, 1.1000, 1.1015);
    add_bar(bars, ts, 1.1015, 1.1040, 1.1010, 1.1035);
    add_bar(bars, ts, 1.1035, 1.1052, 1.1030, 1.1045);
    add_bar(bars, ts, 1.1045, 1.1051, 1.1020, 1.1025);
    add_bar(bars, ts, 1.1025, 1.1035, 1.1015, 1.1020);
    add_bar(bars, ts, 1.1020, 1.1045, 1.1015, 1.1040);
    add_bar(bars, ts, 1.1040, 1.1052, 1.1030, 1.1045);
    add_bar(bars, ts, 1.1045, 1.1051, 1.1025, 1.1030);
    add_bar(bars, ts, 1.1030, 1.1048, 1.1025, 1.1042);
    add_bar(bars, ts, 1.1042, 1.1052, 1.1035, 1.1047);
    add_bar(bars, ts, 1.1047, 1.1051, 1.1030, 1.1035);
    add_bar(bars, ts, 1.1035, 1.1070, 1.1035, 1.1065);
    add_bar(bars, ts, 1.1065, 1.1080, 1.1060, 1.1075);
    add_bar(bars, ts, 1.1075, 1.1080, 1.1050, 1.1055);
    add_bar(bars, ts, 1.1055, 1.1090, 1.1050, 1.1085);
    add_bar(bars, ts, 1.1085, 1.1100, 1.1080, 1.1095);
    return bars;
}

// Уровень снизу, 3 касания, пробой вниз, ретест, отбой.
static std::vector<core::Bar> make_mirror_down() {
    std::vector<core::Bar> bars;
    int64_t ts = 1'700'000'000'000LL;
    add_bar(bars, ts, 1.1000, 1.1010, 1.0995, 1.1005);
    add_bar(bars, ts, 1.1005, 1.1010, 1.0985, 1.0995);
    add_bar(bars, ts, 1.0995, 1.1000, 1.0965, 1.0975);
    add_bar(bars, ts, 1.0975, 1.0985, 1.0948, 1.0955);
    add_bar(bars, ts, 1.0955, 1.0970, 1.0949, 1.0965);
    add_bar(bars, ts, 1.0965, 1.0980, 1.0955, 1.0970);
    add_bar(bars, ts, 1.0970, 1.0980, 1.0950, 1.0955);
    add_bar(bars, ts, 1.0955, 1.0970, 1.0948, 1.0960);
    add_bar(bars, ts, 1.0960, 1.0975, 1.0950, 1.0965);
    add_bar(bars, ts, 1.0965, 1.0975, 1.0949, 1.0955);
    add_bar(bars, ts, 1.0955, 1.0965, 1.0948, 1.0952);
    add_bar(bars, ts, 1.0952, 1.0965, 1.0950, 1.0960);
    add_bar(bars, ts, 1.0960, 1.0960, 1.0930, 1.0935);
    add_bar(bars, ts, 1.0935, 1.0940, 1.0920, 1.0925);
    add_bar(bars, ts, 1.0925, 1.0950, 1.0920, 1.0945);
    add_bar(bars, ts, 1.0945, 1.0950, 1.0910, 1.0915);
    add_bar(bars, ts, 1.0915, 1.0920, 1.0900, 1.0905);
    add_bar(bars, ts, 1.0905, 1.0910, 1.0890, 1.0895);
    add_bar(bars, ts, 1.0895, 1.0900, 1.0880, 1.0885);
    return bars;
}

int main() {
    // 1. Пустой вход.
    {
        cluster::MirrorLevelDetector d;
        const auto r = d.find({});
        check(!r.ok, "empty input -> ok=false");
    }

    // 2. Один бар.
    {
        cluster::MirrorLevelDetector d;
        std::vector<core::Bar> b(1);
        const auto r = d.find(b);
        check(!r.ok, "single bar -> ok=false");
    }

    // 3. Формация вверх — детектор находит сигнал.
    {
        auto bars = make_mirror_up();
        cluster::MirrorLevelDetector d;
        const auto r = d.find(bars);
        check(r.ok, "mirror found");
        check(r.touches_before_break >= 3, "touches >= 3");
        check(r.retest_ok, "retest ok");
        check(r.bars_since_break > 0, "bars_since_break > 0");
        check(r.level > 0.0, "level > 0");
    }

    // 4. Формация вниз — детектор находит сигнал.
    {
        auto bars = make_mirror_down();
        cluster::MirrorLevelDetector d;
        const auto r = d.find(bars);
        check(r.ok, "mirror down found");
        check(r.touches_before_break >= 3, "touches >= 3 down");
    }

    // 5. Монотонный тренд — валидной формации нет.
    {
        std::vector<core::Bar> bars;
        int64_t ts = 1'700'000'000'000LL;
        double p = 1.1000;
        for (int i = 0; i < 30; ++i) {
            core::Bar b;
            b.timestamp = ts;
            b.open = p; b.high = p + 0.0005; b.low = p - 0.0005; b.close = p + 0.0003;
            b.ask_volume = 100; b.bid_volume = 100;
            bars.push_back(b);
            p += 0.0003;
            ts += 300'000LL;
        }
        cluster::MirrorLevelDetector d;
        const auto r = d.find(bars);
        check(!r.ok, "monotonic trend -> no mirror");
    }

    // 6. find_last не падает на длинном потоке.
    {
        auto bars = make_mirror_up();
        int64_t ts = bars.back().timestamp + 300'000LL;
        double p = bars.back().close;
        for (int i = 0; i < 200; ++i) {
            core::Bar b;
            b.timestamp = ts;
            b.open = p; b.high = p + 0.0005; b.low = p - 0.0005; b.close = p + 0.0001;
            b.ask_volume = 100; b.bid_volume = 100;
            bars.push_back(b);
            p += 0.0001;
            ts += 300'000LL;
        }
        cluster::MirrorLevelDetector d;
        const auto r = d.find_last(bars);
        (void)r;
        check(true, "find_last doesn't crash");
    }

    std::cout << "Checks: " << g_checks << ", Failures: " << g_fails << "\n";
    return g_fails == 0 ? 0 : 1;
}