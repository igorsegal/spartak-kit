// test_composite_signal.cpp
#include "cluster/CompositeSignal.h"
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

int main() {
    // 1. Пустой вход.
    {
        cluster::CompositeSignal c;
        const auto r = c.find({});
        check(!r.ok, "empty -> ok=false");
    }

    // 2. Мало баров.
    {
        std::vector<core::Bar> bars;
        int64_t ts = 1'700'000'000'000LL;
        for (int i = 0; i < 10; ++i) {
            add_bar(bars, ts, 1.1, 1.1005, 1.0995, 1.1001);
        }
        cluster::CompositeSignal c;
        const auto r = c.find(bars);
        check(!r.ok, "few bars -> ok=false");
    }

    // 3. Тренд вверх, 200 баров. Не ожидаем ok=true.
    {
        std::vector<core::Bar> bars;
        int64_t ts = 1'700'000'000'000LL;
        double p = 1.1000;
        for (int i = 0; i < 200; ++i) {
            const double o = p;
            const double c = p + 0.0003;
            add_bar(bars, ts, o, c + 0.0002, o - 0.0002, c);
            p = c;
        }
        cluster::CompositeSignal c;
        const auto r = c.find(bars);
        // Проверяем что не падает, что weight посчитан.
        check(r.total_weight >= 0, "trend: total_weight >= 0");
        check(r.opposite_weight >= 0, "trend: opposite_weight >= 0");
    }

    // 4. Ровный флэт.
    {
        std::vector<core::Bar> bars;
        int64_t ts = 1'700'000'000'000LL;
        for (int i = 0; i < 200; ++i) {
            add_bar(bars, ts, 1.1000, 1.1001, 1.0999, 1.1000);
        }
        cluster::CompositeSignal c;
        const auto r = c.find(bars);
        check(r.total_weight >= 0, "flat: total_weight >= 0");
    }

    // 5. Валидация параметров.
    {
        cluster::CompositeOptions opts;
        opts.min_weight = 0;
        cluster::CompositeSignal c(opts);
        std::vector<core::Bar> bars;
        int64_t ts = 1'700'000'000'000LL;
        for (int i = 0; i < 50; ++i) {
            add_bar(bars, ts, 1.1, 1.1005, 1.0995, 1.1001);
        }
        const auto r = c.find(bars);
        check(!r.ok, "invalid opts -> ok=false");
    }

    std::cout << "Checks: " << g_checks << ", Failures: " << g_fails << "\n";
    return g_fails == 0 ? 0 : 1;
}