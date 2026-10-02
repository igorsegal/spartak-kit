// =============================================================================
//  spartak-kit :: tests/test_cluster_csv.cpp
//  Smoke-тест ClusterCsvReader на реальном формате ClusterDelta.
//  Использование: test_cluster_csv <path-to-csv>
// =============================================================================
#include "data/ClusterCsvReader.h"
#include <cmath>
#include <cstdint>
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
static bool approx(double a, double b, double eps = 1e-9) {
    return std::fabs(a - b) < eps;
}
int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "usage: test_cluster_csv <path-to-csv>\n";
        return 2;
    }
    data::ClusterCsvReader reader;
    if (!reader.open(argv[1])) {
        std::cerr << "cannot open " << argv[1] << "\n";
        return 2;
    }
    std::vector<core::Bar> bars;
    core::Bar b;
    while (reader.read_next(b)) bars.push_back(b);
    CHECK(bars.size() == 5);
    CHECK(reader.bars_read() == 5);
    if (bars.size() == 5) {
        // 02.01.2026 01:00 UTC = 1767315600000 ms
        const int64_t t0 = 1767315600000LL;
        CHECK(bars[0].timestamp == t0);
        CHECK(approx(bars[0].open,  1.2719));
        CHECK(approx(bars[0].high,  1.2719));
        CHECK(approx(bars[0].low,   1.2713));
        CHECK(approx(bars[0].close, 1.2717));
        CHECK(bars[0].volume     == 12);
        CHECK(bars[0].delta      ==  4);
        CHECK(bars[0].ask_volume ==  8);
        CHECK(bars[0].bid_volume ==  4);
        CHECK(bars[0].spread     ==  0);
        // 02.01.2026 01:04 UTC = t0 + 4 минуты
        CHECK(bars[4].timestamp == t0 + 4LL * 60000LL);
        CHECK(bars[4].volume == 3);
        CHECK(bars[4].delta  == -3);
    }
    std::cout << "Checks: " << g_checks
              << ", Failures: " << g_failures << "\n";
    return g_failures == 0 ? 0 : 1;
}