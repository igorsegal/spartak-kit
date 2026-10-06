// test_delta_detector.cpp
#include "cluster/DeltaDetector.h"
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
                    double o, double h, double l, double c,
                    double delta, double ask_v, double bid_v) {
    core::Bar b;
    b.timestamp = ts;
    b.open = o; b.high = h; b.low = l; b.close = c;
    b.delta = delta;
    b.ask_volume = ask_v;
    b.bid_volume = bid_v;
    bars.push_back(b);
    ts += 300'000LL;
}

// 30 спокойных баров с дельтой около 0, потом аномалия.
static std::vector<core::Bar> make_calm_then_anomaly(double anomaly_delta,
                                                     bool candle_up) {
    std::vector<core::Bar> bars;
    int64_t ts = 1'700'000'000'000LL;
    double price = 1.1000;
    for (int i = 0; i < 30; ++i) {
        // Спокойная дельта в пределах ±5, случайное чередование.
        const double d = (i % 2 == 0) ? 3.0 : -3.0;
        add_bar(bars, ts, price, price + 0.0005, price - 0.0005,
                price + 0.0001, d, 100.0 + d, 100.0 - d);
        price += 0.0001;
    }
    // Аномальный бар.
    const double o = price;
    const double c = candle_up ? (price + 0.0020) : (price - 0.0020);
    const double h = (o > c ? o : c) + 0.0005;
    const double l = (o < c ? o : c) - 0.0005;
    add_bar(bars, ts, o, h, l, c, anomaly_delta, 100.0 + anomaly_delta, 100.0);
    return bars;
}

int main() {
    // 1. Пустой вход.
    {
        cluster::DeltaDetector d;
        const auto r = d.find({});
        check(!r.ok, "empty input -> ok=false");
    }

    // 2. Мало баров (меньше min_bars).
    {
        std::vector<core::Bar> bars;
        int64_t ts = 1'700'000'000'000LL;
        for (int i = 0; i < 5; ++i) {
            add_bar(bars, ts, 1.1, 1.1005, 1.0995, 1.1001, 5.0, 100.0, 95.0);
        }
        cluster::DeltaDetector d;
        const auto r = d.find(bars);
        check(!r.ok, "too few bars -> ok=false");
    }

    // 3. Аномалия положительной дельты (толпа покупает).
    {
        auto bars = make_calm_then_anomaly(50.0, true);
        cluster::DeltaDetector d;
        const auto r = d.find(bars);
        check(r.ok, "anomaly up found");
        check(r.direction == core::OrderSide::Buy, "direction Buy");
        check(r.anomaly_ratio >= 1.0, "anomaly ratio >= 1");
        check(!r.contradiction, "no contradiction (delta+ and candle up)");
    }

    // 4. Аномалия отрицательной дельты (толпа продаёт).
    {
        auto bars = make_calm_then_anomaly(-50.0, false);
        cluster::DeltaDetector d;
        const auto r = d.find(bars);
        check(r.ok, "anomaly down found");
        check(r.direction == core::OrderSide::Sell, "direction Sell");
        check(r.anomaly_ratio >= 1.0, "anomaly ratio >= 1");
        check(!r.contradiction, "no contradiction (delta- and candle down)");
    }

    // 5. Противоречие: толпа покупает, а свеча падает.
    {
        auto bars = make_calm_then_anomaly(50.0, false);
        cluster::DeltaDetector d;
        const auto r = d.find(bars);
        check(r.ok, "anomaly found in contradiction");
        check(r.contradiction, "contradiction: delta+ but candle down");
    }

    // 6. Спокойный поток без аномалий.
    {
        std::vector<core::Bar> bars;
        int64_t ts = 1'700'000'000'000LL;
        double price = 1.1000;
        for (int i = 0; i < 40; ++i) {
            const double dv = (i % 2 == 0) ? 3.0 : -3.0;
            add_bar(bars, ts, price, price + 0.0005, price - 0.0005,
                    price + 0.0001, dv, 100.0 + dv, 100.0 - dv);
            price += 0.0001;
        }
        cluster::DeltaDetector d;
        const auto r = d.find(bars);
        check(!r.ok, "calm flow -> ok=false");
    }

    // 7. find_at на конкретном индексе.
    {
        auto bars = make_calm_then_anomaly(50.0, true);
        cluster::DeltaDetector d;
        const auto r = d.find_at(bars, bars.size() - 1);
        check(r.ok, "find_at last index -> ok");
    }

    // 8. find_at с индексом за пределами.
    {
        auto bars = make_calm_then_anomaly(50.0, true);
        cluster::DeltaDetector d;
        const auto r = d.find_at(bars, 9999);
        check(!r.ok, "find_at out of bounds -> ok=false");
    }

    std::cout << "Checks: " << g_checks << ", Failures: " << g_fails << "\n";
    return g_fails == 0 ? 0 : 1;
}