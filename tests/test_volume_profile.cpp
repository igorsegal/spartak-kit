// test_volume_profile.cpp
#include "cluster/VolumeProfileFilter.h"
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

static core::Bar make_bar(int64_t ts, double o, double h, double l, double c,
                          double ask_v, double bid_v) {
    core::Bar b;
    b.timestamp  = ts;
    b.open = o; b.high = h; b.low = l; b.close = c;
    b.ask_volume = ask_v;
    b.bid_volume = bid_v;
    b.volume = ask_v + bid_v;
    b.delta = ask_v - bid_v;
    return b;
}

// Покупателей больше (ask доминирует). Ожидается Sell.
static std::vector<core::Bar> make_ask_dominant() {
    std::vector<core::Bar> bars;
    int64_t ts = 1'700'000'000'000LL;
    double p = 1.1000;
    for (int i = 0; i < 30; ++i) {
        const double o = p;
        const double c = p + 0.0003;
        bars.push_back(make_bar(ts, o, c + 0.0002, o - 0.0002, c, 200.0, 50.0));
        p = c;
        ts += 300'000LL;
    }
    return bars;
}

// Продавцов больше (bid доминирует). Ожидается Buy.
static std::vector<core::Bar> make_bid_dominant() {
    std::vector<core::Bar> bars;
    int64_t ts = 1'700'000'000'000LL;
    double p = 1.1000;
    for (int i = 0; i < 30; ++i) {
        const double o = p;
        const double c = p - 0.0003;
        bars.push_back(make_bar(ts, o, o + 0.0002, c - 0.0002, c, 50.0, 200.0));
        p = c;
        ts += 300'000LL;
    }
    return bars;
}

// Нейтральный — ask ≈ bid.
static std::vector<core::Bar> make_neutral() {
    std::vector<core::Bar> bars;
    int64_t ts = 1'700'000'000'000LL;
    double p = 1.1000;
    for (int i = 0; i < 30; ++i) {
        const double o = p;
        const double c = p + ((i % 2 == 0) ? 0.0002 : -0.0002);
        bars.push_back(make_bar(ts, o, o + 0.0004, o - 0.0004, c, 100.0, 100.0));
        p = o;
        ts += 300'000LL;
    }
    return bars;
}

int main() {
    // 1. Пустой вход.
    {
        cluster::VolumeProfileFilter d;
        const auto r = d.find({});
        check(!r.ok, "empty input -> ok=false");
    }

    // 2. Слишком мало баров.
    {
        std::vector<core::Bar> bars;
        int64_t ts = 1'700'000'000'000LL;
        for (int i = 0; i < 10; ++i) {
            bars.push_back(make_bar(ts, 1.1000, 1.1005, 1.0995, 1.1001, 100.0, 100.0));
            ts += 300'000LL;
        }
        cluster::VolumeProfileFilter d;
        const auto r = d.find(bars);
        check(!r.ok, "too few bars -> ok=false");
    }

    // 3. Все бары одинаковые (диапазон = 0).
    {
        std::vector<core::Bar> bars;
        int64_t ts = 1'700'000'000'000LL;
        for (int i = 0; i < 30; ++i) {
            bars.push_back(make_bar(ts, 1.1000, 1.1000, 1.1000, 1.1000, 100.0, 100.0));
            ts += 300'000LL;
        }
        cluster::VolumeProfileFilter d;
        const auto r = d.find(bars);
        check(!r.ok, "flat range -> ok=false");
    }

    // 4. Покупателей больше → Sell.
    {
        auto bars = make_ask_dominant();
        cluster::VolumeProfileFilter d;
        const auto r = d.find(bars);
        check(r.ok, "ask dominant -> ok=true");
        if (r.ok) {
            check(r.direction == core::OrderSide::Sell, "ask dominant -> Sell");
            check(r.imbalance > 0.0, "imbalance > 0");
            check(r.ask_total > r.bid_total, "ask_total > bid_total");
            check(r.bins_used >= 2, "bins_used >= 2");
        }
    }

    // 5. Продавцов больше → Buy.
    {
        auto bars = make_bid_dominant();
        cluster::VolumeProfileFilter d;
        const auto r = d.find(bars);
        check(r.ok, "bid dominant -> ok=true");
        if (r.ok) {
            check(r.direction == core::OrderSide::Buy, "bid dominant -> Buy");
            check(r.imbalance < 0.0, "imbalance < 0");
            check(r.bid_total > r.ask_total, "bid_total > ask_total");
        }
    }

    // 6. Нейтральный → ok=false.
    {
        auto bars = make_neutral();
        cluster::VolumeProfileFilter d;
        const auto r = d.find(bars);
        check(!r.ok, "neutral -> ok=false");
    }

    // 7. Value area вокруг POC.
    {
        auto bars = make_ask_dominant();
        cluster::VolumeProfileFilter d;
        const auto r = d.find(bars);
        check(r.ok, "va check -> ok");
        if (r.ok) {
            check(r.value_area_high > r.value_area_low, "va_high > va_low");
            check(r.poc >= r.value_area_low && r.poc <= r.value_area_high, "poc inside va");
        }
    }

    // 8. Бар с NaN пропускается.
    {
        auto bars = make_ask_dominant();
        core::Bar bad = bars[5];
        bad.high = std::nan("");
        bars[5] = bad;
        cluster::VolumeProfileFilter d;
        const auto r = d.find(bars);
        check(r.ok, "nan bar skipped, still ok");
    }

    // 9. Бар с volume > 0, ask+bid == 0 пропускается.
    {
        auto bars = make_ask_dominant();
        core::Bar bad = bars[10];
        bad.ask_volume = 0.0;
        bad.bid_volume = 0.0;
        bad.volume = 100.0;
        bars[10] = bad;
        cluster::VolumeProfileFilter d;
        const auto r = d.find(bars);
        check(r.ok, "volume>0, ask+bid==0 skipped");
    }

    std::cout << "Checks: " << g_checks << ", Failures: " << g_fails << "\n";
    return g_fails == 0 ? 0 : 1;
}