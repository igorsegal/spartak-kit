#include "cluster/MirrorLevelDetector.h"
#include <iostream>
#include <vector>
using namespace spartak;

int main() {
    std::vector<core::Bar> bars;
    int64_t ts = 1'700'000'000'000LL;

    auto add = [&](double o, double h, double l, double c) {
        core::Bar b;
        b.timestamp = ts; b.open = o; b.high = h; b.low = l; b.close = c;
        b.ask_volume = 100; b.bid_volume = 100;
        bars.push_back(b);
        ts += 300'000LL;
    };

    add(1.1000, 1.1010, 1.0995, 1.1005);
    add(1.1005, 1.1020, 1.1000, 1.1015);
    add(1.1015, 1.1040, 1.1010, 1.1035);

    add(1.1035, 1.1052, 1.1030, 1.1045);
    add(1.1045, 1.1051, 1.1020, 1.1025);
    add(1.1025, 1.1035, 1.1015, 1.1020);

    add(1.1020, 1.1045, 1.1015, 1.1040);
    add(1.1040, 1.1052, 1.1030, 1.1045);
    add(1.1045, 1.1051, 1.1025, 1.1030);

    add(1.1030, 1.1048, 1.1025, 1.1042);
    add(1.1042, 1.1052, 1.1035, 1.1047);
    add(1.1047, 1.1051, 1.1030, 1.1035);

    add(1.1035, 1.1070, 1.1035, 1.1065);
    add(1.1065, 1.1080, 1.1060, 1.1075);

    add(1.1075, 1.1080, 1.1050, 1.1055);

    add(1.1055, 1.1090, 1.1050, 1.1085);
    add(1.1085, 1.1100, 1.1080, 1.1095);

    std::cout << "Total bars: " << bars.size() << "\n";

    const double test_level = 1.1050;
    const double range_val = 1.1100 - 1.0995;
    const double test_tol = range_val * 0.05;

    std::cout << "price_range=" << range_val << " tol=" << test_tol << "\n";
    std::cout << "touch zone: [" << test_level - test_tol << ", " << test_level + test_tol << "]\n";

    for (size_t i = 0; i < bars.size(); ++i) {
        const auto& b = bars[i];
        bool t = b.low <= test_level + test_tol && b.high >= test_level - test_tol;
        if (t) {
            std::cout << "[" << i << "] TOUCH level " << test_level << "\n";
        }
    }

    cluster::MirrorLevelDetector d;
    const auto r = d.find(bars);

    std::cout << "\nResult:\n";
    std::cout << "  ok=" << r.ok << "\n";
    std::cout << "  direction=" << (int)r.direction << "\n";
    std::cout << "  level=" << r.level << "\n";
    std::cout << "  touches_before_break=" << r.touches_before_break << "\n";
    std::cout << "  retest_ok=" << r.retest_ok << "\n";
    std::cout << "  bars_since_break=" << r.bars_since_break << "\n";

    return 0;
}