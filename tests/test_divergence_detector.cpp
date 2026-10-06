// test_divergence_detector.cpp
#include "cluster/DivergenceDetector.h"
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

// Медвежья дивергенция: два максимума, второй выше, RSI падает.
static std::vector<core::Bar> make_bearish_divergence() {
    std::vector<core::Bar> bars;
    int64_t ts = 1'700'000'000'000LL;
    double p = 1.1000;

    for (int i = 0; i < 15; ++i) {
        const double o = p;
        const double c = p + 0.0010;
        add_bar(bars, ts, o, c + 0.0002, o - 0.0002, c);
        p = c;
    }
    // Первый максимум.
    add_bar(bars, ts, p, p + 0.0050, p - 0.0002, p + 0.0040);
    p += 0.0040;

    // Откат.
    for (int i = 0; i < 15; ++i) {
        const double o = p;
        const double c = p - 0.0008;
        add_bar(bars, ts, o, o + 0.0002, c - 0.0002, c);
        p = c;
    }

    // Медленный рост ко второму максимуму.
    for (int i = 0; i < 15; ++i) {
        const double o = p;
        const double c = p + 0.0005;
        add_bar(bars, ts, o, c + 0.0002, o - 0.0002, c);
        p = c;
    }
    // Второй максимум.
    add_bar(bars, ts, p, p + 0.0060, p - 0.0002, p + 0.0050);
    p += 0.0050;

    // Откат.
    for (int i = 0; i < 10; ++i) {
        const double o = p;
        const double c = p - 0.0004;
        add_bar(bars, ts, o, o + 0.0002, c - 0.0002, c);
        p = c;
    }

    return bars;
}

// Бычья дивергенция: два минимума, второй ниже, RSI выше.
static std::vector<core::Bar> make_bullish_divergence() {
    std::vector<core::Bar> bars;
    int64_t ts = 1'700'000'000'000LL;
    double p = 1.2000;

    for (int i = 0; i < 15; ++i) {
        const double o = p;
        const double c = p - 0.0010;
        add_bar(bars, ts, o, o + 0.0002, c - 0.0002, c);
        p = c;
    }
    // Первый минимум.
    add_bar(bars, ts, p, p + 0.0002, p - 0.0050, p - 0.0040);
    p -= 0.0040;

    // Откат вверх.
    for (int i = 0; i < 15; ++i) {
        const double o = p;
        const double c = p + 0.0008;
        add_bar(bars, ts, o, c + 0.0002, o - 0.0002, c);
        p = c;
    }

    // Медленное падение ко второму минимуму.
    for (int i = 0; i < 15; ++i) {
        const double o = p;
        const double c = p - 0.0005;
        add_bar(bars, ts, o, o + 0.0002, c - 0.0002, c);
        p = c;
    }
    // Второй минимум.
    add_bar(bars, ts, p, p + 0.0002, p - 0.0060, p - 0.0050);
    p -= 0.0050;

    // Откат вверх.
    for (int i = 0; i < 10; ++i) {
        const double o = p;
        const double c = p + 0.0004;
        add_bar(bars, ts, o, c + 0.0002, o - 0.0002, c);
        p = c;
    }

    return bars;
}

int main() {
    {
        cluster::DivergenceDetector d;
        const auto r = d.find({});
        check(!r.ok, "empty input -> ok=false");
    }

    {
        std::vector<core::Bar> bars;
        int64_t ts = 1'700'000'000'000LL;
        for (int i = 0; i < 10; ++i) {
            add_bar(bars, ts, 1.1, 1.1005, 1.0995, 1.1001);
        }
        cluster::DivergenceDetector d;
        const auto r = d.find(bars);
        check(!r.ok, "too few bars -> ok=false");
    }

    {
        auto bars = make_bearish_divergence();
        cluster::DivergenceDetector d;
        const auto r = d.find(bars);
        check(r.ok, "bearish divergence found");
        check(r.direction == core::OrderSide::Sell, "direction Sell");
        check(r.price_pivot2 > r.price_pivot1, "price2 > price1");
        check(r.rsi_pivot2 < r.rsi_pivot1, "rsi2 < rsi1");
        check(r.bars_between > 0, "bars_between > 0");
    }

    {
        auto bars = make_bullish_divergence();
        cluster::DivergenceDetector d;
        const auto r = d.find(bars);
        check(r.ok, "bullish divergence found");
        check(r.direction == core::OrderSide::Buy, "direction Buy");
        check(r.price_pivot2 < r.price_pivot1, "price2 < price1");
        check(r.rsi_pivot2 > r.rsi_pivot1, "rsi2 > rsi1");
    }

    {
        std::vector<core::Bar> bars;
        int64_t ts = 1'700'000'000'000LL;
        double p = 1.1000;
        for (int i = 0; i < 60; ++i) {
            const double o = p;
            const double c = p + 0.0005;
            add_bar(bars, ts, o, c + 0.0002, o - 0.0002, c);
            p = c;
        }
        cluster::DivergenceDetector d;
        const auto r = d.find(bars);
        check(!r.ok, "monotonic up -> no divergence");
    }

    std::cout << "Checks: " << g_checks << ", Failures: " << g_fails << "\n";
    return g_fails == 0 ? 0 : 1;
}