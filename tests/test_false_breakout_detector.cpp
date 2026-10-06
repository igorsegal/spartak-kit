#include "cluster/FalseBreakoutDetector.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <vector>
using namespace spartak;

static void add_bar(std::vector<core::Bar>& bars, int64_t& ts,
                    double o, double h, double l, double c) {
    core::Bar b;
    b.timestamp = ts;
    b.open = o; b.high = h; b.low = l; b.close = c;
    b.ask_volume = 100; b.bid_volume = 100;
    bars.push_back(b);
    ts += 300'000LL;
}

int main() {
    std::vector<core::Bar> bars;
    int64_t ts = 1'700'000'000'000LL;

    for (int i = 0; i < 30; ++i) {
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

    std::cout << "Bars: " << bars.size() << "\n";

    double mn = bars[0].low, mx = bars[0].high;
    for (const auto& b : bars) {
        if (b.low < mn) mn = b.low;
        if (b.high > mx) mx = b.high;
    }
    double range = mx - mn;
    double tol = range * 0.01;
    std::cout << "min_low=" << mn << " max_high=" << mx << " range=" << range << " tol=" << tol << "\n";
    std::cout << "zone for 1.1050: [" << 1.1050 - tol << ", " << 1.1050 + tol << "]\n";

    int touches = 0;
    for (size_t i = 0; i < bars.size(); ++i) {
        if (bars[i].low <= 1.1050 + tol && bars[i].high >= 1.1050 - tol) {
            ++touches;
            std::cout << "  touch at [" << i << "] H=" << bars[i].high << " L=" << bars[i].low << " C=" << bars[i].close << "\n";
        }
    }
    std::cout << "touches of 1.1050: " << touches << "\n";

    cluster::FalseBreakoutDetector d;
    const auto r = d.find(bars);
    std::cout << "Result: ok=" << r.ok << " dir=" << (int)r.direction
              << " level=" << r.level << " touches=" << r.touches_before
              << " bars_outside=" << r.bars_outside << "\n";

    return 0;
}