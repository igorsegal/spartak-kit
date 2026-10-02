// =============================================================================
//  spartak-kit :: tests/smoke_test.cpp
//  Минимальные smoke-тесты без внешнего фреймворка.
// =============================================================================
#include "data/DataSanitizer.h"
#include "data/BarStream.h"
#include <iostream>
#include <vector>
using namespace spartak;
static int g_checks   = 0;
static int g_failures = 0;
#define CHECK(cond) do {                                              \
    ++g_checks;                                                       \
    if (!(cond)) {                                                    \
        ++g_failures;                                                 \
        std::cerr << "FAIL line " << __LINE__ << ": " #cond "\n";     \
    }                                                                 \
} while (0)
static std::vector<core::Bar> make_bars(int count, int64_t start_ms, int64_t step_ms) {
    std::vector<core::Bar> v;
    v.reserve(static_cast<std::size_t>(count));
    for (int i = 0; i < count; ++i) {
        core::Bar b;
        b.timestamp   = start_ms + static_cast<int64_t>(i) * step_ms;
        b.open        = b.high = b.low = b.close = 1.1000 + i * 0.0001;
        b.volume = 100;
        b.spread      = 12;
        v.push_back(b);
    }
    return v;
}
static void test_empty_stream() {
    data::BarStream s{std::vector<core::Bar>{}};
    data::DataSanitizer ds;
    auto r = ds.run(s);
    CHECK(!r.ok);
    CHECK(r.bars_scanned == 0);
}
static void test_regular_stream() {
    auto bars = make_bars(20, 1'700'000'000'000LL, 3'600'000LL);
    data::BarStream s{bars};
    data::DataSanitizer ds(14'400'000LL, 10);
    auto r = ds.run(s);
    CHECK(r.ok);
    CHECK(r.bars_skipped == 0);
    CHECK(r.first_regular_timestamp == bars[0].timestamp);
    int n = 0;
    core::Bar b;
    while (s.next(b)) ++n;
    CHECK(n == 20);
}
static void test_irregular_prefix() {
    std::vector<core::Bar> bars;
    auto daily  = make_bars(5,  1'700'000'000'000LL,                     86'400'000LL);
    auto hourly = make_bars(20, 1'700'000'000'000LL + 5LL*86'400'000LL,  3'600'000LL);
    bars.insert(bars.end(), daily.begin(),  daily.end());
    bars.insert(bars.end(), hourly.begin(), hourly.end());
    data::BarStream s{bars};
    data::DataSanitizer ds(14'400'000LL, 10);
    auto r = ds.run(s);
    CHECK(r.ok);
    CHECK(r.bars_skipped >= 5);
    CHECK(r.first_regular_timestamp >= hourly[0].timestamp - 3'600'000LL);
}
static void test_no_regular_tail() {
    auto bars = make_bars(30, 1'700'000'000'000LL, 86'400'000LL);
    data::BarStream s{bars};
    data::DataSanitizer ds(14'400'000LL, 10);
    auto r = ds.run(s);
    CHECK(!r.ok);
    CHECK(r.bars_skipped == 30);
}
static void test_synthetic_mode() {
    data::BarStream::SyntheticParams p;
    p.max_bars = 50;
    data::BarStream s{p};
    data::DataSanitizer ds(14'400'000LL, 10);
    auto r = ds.run(s);
    CHECK(r.ok);
    CHECK(r.bars_skipped == 0);
}
int main() {
    test_empty_stream();
    test_regular_stream();
    test_irregular_prefix();
    test_no_regular_tail();
    test_synthetic_mode();
    std::cout << "Checks: " << g_checks
              << ", Failures: " << g_failures << "\n";
    return g_failures == 0 ? 0 : 1;
}